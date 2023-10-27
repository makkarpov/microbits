from typing import Optional
import asyncio


ENV_AGGREGATOR_PORT = 'UB_TEST_AGGREGATOR_PORT'


class MessageWriter:
    def __init__(self, wr: asyncio.StreamWriter):
        self._wr = wr

    async def write_bytes(self, b: bytes):
        from struct import pack
        self._wr.write(pack('>I', len(b)))
        self._wr.write(b)

        await self._wr.drain()

    async def write_msg(self, msg):
        from pickle import dumps
        await self.write_bytes(dumps(msg))


class MessageReader:
    _rd: asyncio.StreamReader
    _data: Optional[bytes]

    def __init__(self, rd: asyncio.StreamReader):
        self._rd = rd
        self._data = None

    async def read_next(self) -> bool:
        from struct import unpack
        self._data = None

        try:
            header = await self._rd.readexactly(4)
        except asyncio.IncompleteReadError as e:
            if len(e.partial) != 0:
                raise e

            return False

        length, = unpack('>I', header)

        self._data = await self._rd.readexactly(length)
        return True

    @property
    def data(self) -> bytes:
        if self._data is None:
            raise RuntimeError('no data is present in reader')

        return self._data

    @property
    def message(self):
        from pickle import loads
        return loads(self.data)

