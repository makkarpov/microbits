import asyncio
import sys
from typing import List

import aggregator_util as _au
import aggregator_msg as _am


class TestAggregator:
    _server: asyncio.Server
    _results: List[_am.ResultMessage]

    async def _handle_client(self, reader: asyncio.StreamReader, writer: asyncio.StreamWriter):
        mr = _au.MessageReader(reader)
        mw = _au.MessageWriter(writer)

        try:
            while await mr.read_next():
                msg = mr.message

                if isinstance(msg, _am.ResultMessage):
                    self._results.append(msg)
        except Exception:
            import traceback
            print('Error while handling client')
            print(traceback.format_exc())
        finally:
            writer.close()
            await writer.wait_closed()

    def _server_port(self) -> int:
        socket = self._server.sockets[0]
        return int(socket.getsockname()[1])

    async def _run_tests(self):
        proc = await asyncio.create_subprocess_shell(
            'ctest --output-on-failure',
            env={
                _au.ENV_AGGREGATOR_PORT: str(self._server_port())
            }
        )

        await proc.wait()
        if proc.returncode != 0:
            raise RuntimeError(f'ctest process exited with code {proc.returncode}')

    def _print_results(self):
        from prettytable import SINGLE_BORDER
        from prettytable.colortable import ColorTable, Theme, RESET_CODE

        for x in self._results:
            if x.success:
                continue

            print('')
            print('### %s - FAILURE' % x.name)
            print('=' * 120)
            print(x.failure)
            print('=' * 120)

        tbl = ColorTable()
        tbl.set_style(SINGLE_BORDER)
        tbl.field_names = ['Test', 'Result', 'Code', 'Stack', 'Cycles', 'Time (s)']

        tbl.align['Code'] = 'r'
        tbl.align['Stack'] = 'r'
        tbl.align['Cycles'] = 'r'
        tbl.align['Time (s)'] = 'r'
        tbl.align['Test'] = 'l'

        for x in self._results:
            if x.failure:
                tbl.add_row([
                    x.name,
                    Theme.format_code('31') + 'FAIL' + RESET_CODE,
                    '---', '---', '---', '---'
                ])
                continue

            ex = x.result.execution
            tbl.add_row([
                x.name,
                Theme.format_code('32') + 'OK' + RESET_CODE,
                str(x.result.executable_sz),
                str(ex.stack_bytes),
                str(ex.cycles),
                '%.3f' % ex.cpu_time
            ])

        print(tbl)

    async def run(self):
        self._results = []
        self._server = await asyncio.start_server(self._handle_client, host='127.0.0.1', port=0)

        async with self._server:
            await self._server.start_serving()
            await self._run_tests()

        await self._server.wait_closed()
        self._print_results()

        have_failures = any(not x.success for x in self._results)
        if have_failures:
            sys.exit(1)


if __name__ == '__main__':
    asyncio.run(TestAggregator().run())
