from typing import Union

import swd as _swd


class DebugLink:
    def __init__(self):
        self.swd = _swd.Swd(swd_frequency=16000000)
        self.cortex = _swd.CortexM(self.swd)

        if self.swd.get_idcode() != 0x6BA02477:
            raise RuntimeError('IDCODE check failed (expected STM32H563ZI-NUCLEO board)')

        self.swd.open_ap(1)
        self.swd.default_ap = 1

        self.cortex.reset_halt()

        import time
        t1 = time.time()
        while not self.cortex.is_halted():
            if time.time() - t1 > 1:
                raise RuntimeError('timeout waiting for target to halt')

            time.sleep(0.1)

    def read_mem(self, address: int, length: int) -> bytes:
        r = bytearray(length)
        i = 0

        for b in self.swd.read_mem(address, length):
            r[i] = b
            i += 1

        return bytes(r)

    def read_u32(self, address: int) -> int:
        return self.swd.get_mem32(address)

    def write_mem(self, address: int, data: bytes):
        self.swd.write_mem(address, data)

    def write_u32(self, address: int, value: int):
        self.swd.set_mem32(address, value)

    def modify_u32(self, address: int, v_reset: int, v_set: int):
        v = self.read_u32(address)
        v = (v & ~v_reset) | v_set
        self.write_u32(address, v)

    def read_reg(self, reg: Union[str, int]) -> int:
        return self.cortex.get_reg(reg)

    def write_reg(self, reg: Union[str, int], value: int):
        self.cortex.set_reg(reg, value)

    def fill_mem(self, address: int, size: int, data: bytes):
        self.swd.fill_mem(address, data, size)

    def run(self):
        self.cortex.run()

    def halt(self):
        self.cortex.halt()

    def is_halted(self) -> bool:
        return self.cortex.is_halted()
