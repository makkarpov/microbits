import asyncio
import os
from typing import Union, Optional

from debug_link import DebugLink
from native import Executable, TargetManager
from runner import TestRunner, TestEnvironment

import aggregator_util as _au
import aggregator_msg as _am


class _TestEnvironmentImpl(TestEnvironment):
    def __init__(self, name: str, link: DebugLink, exe: Executable, target: TargetManager):
        self._name = name
        self._link = link
        self._exe = exe
        self._target = target

        self.exec_results = []

    @property
    def link(self) -> DebugLink:
        return self._link

    @property
    def scratchpad_ptr(self) -> int:
        return self._target.ram_scratchpad_ptr

    @property
    def scratchpad_sz(self) -> int:
        return self._target.ram_scratchpad_sz

    def run(self, result_len: int = 0):
        r = self._target.run_executable()
        self.exec_results.append(r)

        print('-' * 80)
        print('Execution completed - %s:' % self._name)
        print('  Executable size:  %d bytes' % self._exe.load_size)
        print('  Processor cycles: %d' % r.cycles)
        print('  Execution time:   %.3fs' % r.cpu_time)
        print('  Peak stack usage: %d bytes' % r.stack_bytes)
        print('-' * 80)

        if result_len == 0:
            return None

        if result_len <= 4:
            return self._link.read_reg('R0')

        if result_len <= 8:
            r = self._link.read_reg('R1') << 32
            r |= self._link.read_reg('R0')
            return r

        raise ValueError('result_len greater than 8 is not supported: %d' % result_len)

    def write(self, location: Union[str, int], data: Union[int, bytes]):
        if isinstance(location, str):
            sym = self._exe.symbol(location)
            addr = sym.address

            if isinstance(data, int):
                if data > 0:
                    data = data.to_bytes(sym.size, 'little')
                else:
                    data = data.to_bytes(sym.size, 'little', signed=True)

            if len(data) > sym.size:
                raise ValueError('written data is bigger than symbol \'%s\': %d > %d' % (sym.name, len(data), sym.size))
        else:
            addr = int(location)
            if not isinstance(data, bytes):
                raise ValueError('data must be raw bytes when writing absolute addresses: %08X' % addr)

        self._link.write_mem(addr, data)

    def read(self, location: Union[str, int], length: Optional[int] = None) -> bytes:
        if isinstance(location, str):
            sym = self._exe.symbol(location)
            addr = sym.address
            length = length or sym.size
        else:
            addr = int(location)
            if length is None:
                raise ValueError('length must be set when reading absolute addresses: %08X' % addr)

        return self._link.read_mem(addr, length)


class _AggregatorConnection:
    _reader: asyncio.StreamReader
    _mr: _au.MessageReader
    _writer: asyncio.StreamWriter
    _mw: _au.MessageWriter

    async def setup(self):
        port = int(os.environ[_au.ENV_AGGREGATOR_PORT])
        self._reader, self._writer = await asyncio.open_connection('127.0.0.1', port)

        self._mr = _au.MessageReader(self._reader)
        self._mw = _au.MessageWriter(self._writer)

    async def write_result(self, msg: _am.ResultMessage):
        await self._mw.write_msg(msg)

    async def finish(self):
        await self._writer.drain()

        self._writer.close()
        await self._writer.wait_closed()


class DeviceTestApp:
    link: DebugLink
    executable: Executable
    target: TargetManager

    _aggregator: Optional[_AggregatorConnection]
    _test_env: _TestEnvironmentImpl
    _runner: TestRunner

    def __init__(self):
        self.args = DeviceTestApp._parse_args()

    @staticmethod
    def _parse_args():
        from argparse import ArgumentParser
        parser = ArgumentParser()

        parser.add_argument('--name')
        parser.add_argument('--executable')
        parser.add_argument('--runner')
        parser.add_argument('--runner_args', nargs='*')

        return parser.parse_args()

    def _load_runner(self) -> TestRunner:
        import importlib.util
        spec = importlib.util.spec_from_file_location('runner_impl', self.args.runner)
        runner = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(runner)

        if not hasattr(runner, 'TestRunnerImpl'):
            raise RuntimeError('No \'TestRunnerImpl\' class in file %s' % self.args.runner)

        return runner.TestRunnerImpl(self._test_env, self.args.runner_args)

    async def run(self):
        if _au.ENV_AGGREGATOR_PORT in os.environ:
            self._aggregator = _AggregatorConnection()
            await self._aggregator.setup()

        result = None
        failure = None

        try:
            result = await self._run_test()
        except Exception:
            import traceback
            failure = traceback.format_exc()

        if self._aggregator is not None:
            result_msg = _am.ResultMessage(
                name=self.args.name,
                success=failure is None,
                failure=failure,
                result=result
            )

            await self._aggregator.write_result(result_msg)

    async def _run_test(self) -> _am.TestResult:
        self.link = DebugLink()
        self.target = TargetManager(self.link)
        self.executable = Executable(self.args.executable)

        self.target.load_executable(self.executable)
        self.target.erase_scratchpad()

        self._test_env = _TestEnvironmentImpl(self.args.name, self.link, self.executable, self.target)
        self._runner = self._load_runner()

        self._runner.run()

        if len(self._test_env.exec_results) == 0:
            raise RuntimeError('no test was executed')

        return _am.TestResult(
            executable_sz=self.executable.load_size,
            execution=self._test_env.exec_results[0]
        )


if __name__ == '__main__':
    asyncio.run(DeviceTestApp().run())
