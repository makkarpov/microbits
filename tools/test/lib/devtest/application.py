import secrets
import struct
from typing import Union, Optional

from debug_link import DebugLink
from native import Executable, TargetManager
from runner import TestRunner, TestEnvironment


class _TestEnvironmentImpl(TestEnvironment):
    def __init__(self, link: DebugLink, exe: Executable, target: TargetManager):
        self._link = link
        self._exe = exe
        self._target = target

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
        self._target.run_executable()

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


class DeviceTestApp:
    link: DebugLink
    executable: Executable
    target: TargetManager
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

    def run(self):
        from debug_link import DebugLink

        self.link = DebugLink()
        self.target = TargetManager(self.link)
        self.executable = Executable(self.args.executable)

        self.target.load_executable(self.executable)
        print('executable size: %d bytes' % self.executable.load_size)

        self.target.erase_scratchpad()

        self._test_env = _TestEnvironmentImpl(self.link, self.executable, self.target)
        self._runner = self._load_runner()

        self._runner.run()


if __name__ == '__main__':
    DeviceTestApp().run()
