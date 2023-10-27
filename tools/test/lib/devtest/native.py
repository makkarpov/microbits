import time
from typing import Optional, NamedTuple

from debug_link import DebugLink


class Regs:
    DEMCR = 0xE000EDFC
    DEMCR_TRCENA = 0x01000000
    DWT_CTRL = 0xE0001000
    DWT_CTRL_CYCCNTENA = 0x00000001
    DWT_CYCCNT = 0xE0001004


class Symbol(NamedTuple):
    name: str
    address: int
    size: int


class ExecutionResult(NamedTuple):
    cycles: int
    cpu_time: float
    stack_bytes: int


class Executable:
    def __init__(self, filename: str):
        from elftools.elf.elffile import ELFFile

        self.filename = filename
        self.file = open(filename, 'rb')
        self.elf = ELFFile(self.file)

        self.min_addr = -1
        self.max_addr = -1
        self.load_size = 0

        for s in self.elf.iter_segments():
            s_min = s.header['p_vaddr']
            s_max = s_min + s.header['p_memsz']

            if s_min == s_max:
                continue

            if self.min_addr < 0:
                self.min_addr = s_min
                self.max_addr = s_max
            else:
                self.min_addr = min(self.min_addr, s_min)
                self.max_addr = max(self.max_addr, s_max)

            self.load_size += s.header['p_filesz']

        self.entry_point = self.elf.header['e_entry']
        self._symbols = self.elf.get_section_by_name('.symtab')

    def symbol(self, name: str) -> Symbol:
        r = self._symbols.get_symbol_by_name(name)
        if r is None or len(r) == 0:
            raise ValueError('symbol not found: %s' % name)

        v = r[0].entry
        return Symbol(name, v['st_value'], v['st_size'])


class TargetManager:
    def __init__(self, link: DebugLink):
        self.link = link

        self.f_cpu = 32000000

        self.ram_start = 0x20000000
        self.ram_total = 640 * 1024

        self.ram_reserved_sz = 4096
        self.ram_stack_sz = 32768

        self.ram_scratchpad_ptr = self.ram_start
        self.ram_reserved_ptr = self.ram_start + self.ram_total - self.ram_reserved_sz
        self.ram_stack_ptr = self.ram_reserved_ptr - self.ram_stack_sz

        self.exe_max_size = self.ram_total - self.ram_stack_sz - self.ram_reserved_sz
        self.exe_entry_point = 0

    @property
    def ram_scratchpad_sz(self) -> int:
        return self.ram_stack_ptr - self.ram_scratchpad_ptr

    def fill_memory(self, start: int, end: int, value: int):
        # void erase_memory(void *start, void *end, uint32_t fill);
        erase_applet = bytes.fromhex('0b1a032b02dc884203d155be40f8042bf6e700f8012bf6e7')

        value = value & 0xFF
        value = value | (value << 8)
        value = value | (value << 16)

        self.link.write_mem(self.ram_reserved_ptr, erase_applet)
        self.link.write_reg('R0', start)
        self.link.write_reg('R1', end)
        self.link.write_reg('R2', value)
        self.link.write_reg('PC', self.ram_reserved_ptr)

        self.link.run()
        while not self.link.is_halted():
            time.sleep(0.05)

    def load_executable(self, exe: Executable):
        if exe.min_addr < self.ram_start or exe.max_addr >= self.ram_start + self.exe_max_size:
            raise RuntimeError('Executable address is out of bounds')

        if exe.min_addr != self.ram_start:
            self.fill_memory(self.ram_start, exe.min_addr, 0x00)

        for s in exe.elf.iter_segments():
            load_addr = s.header['p_paddr']
            load_len = s.header['p_filesz']
            total_len = s.header['p_memsz']

            self.link.write_mem(load_addr, s.data())
            if load_len != total_len:
                self.fill_memory(load_addr + load_len, load_addr + total_len, 0x00)

        self.ram_scratchpad_ptr = exe.max_addr
        self.exe_entry_point = exe.entry_point

    @staticmethod
    def __get_used_stack(ref: bytes, actual: bytes) -> int:
        stack_good = 0
        while stack_good < len(ref) and ref[stack_good] == actual[stack_good]:
            stack_good += 1

        stack_align = 8
        stack_good = (stack_good // stack_align) * stack_align
        return len(ref) - stack_good

    def run_executable(self) -> ExecutionResult:
        import secrets

        # Prepare for stack usage measurement:
        stack_image = secrets.token_bytes(self.ram_stack_sz)
        self.link.write_mem(self.ram_stack_ptr, stack_image)

        # Prepare for cycle counting:
        self.link.modify_u32(Regs.DEMCR, 0, Regs.DEMCR_TRCENA)
        self.link.modify_u32(Regs.DWT_CTRL, 0, Regs.DWT_CTRL_CYCCNTENA)
        self.link.write_u32(Regs.DWT_CYCCNT, 0)

        # Initialize entry point and run:
        self.link.write_mem(self.ram_reserved_ptr, b'\x55\xBE')         # 'bkpt' instruction
        self.link.write_reg('SP', self.ram_stack_ptr + self.ram_stack_sz)
        self.link.write_reg('LR', self.ram_reserved_ptr | 1)            # set thumb bit (to handle 'bx lr')
        self.link.write_reg('PC', self.exe_entry_point & ~1)            # load without thumb bit
        self.link.run()

        cycles_overflow = 0
        cycles_last = 0

        t_start = time.time()
        while not self.link.is_halted():
            if time.time() - t_start >= 30:
                self.link.halt()
                raise RuntimeError('execution timeout')

            # at 160MHz clock frequency 32-bit CYCCNT counter will overflow every 26s
            cycles_now = self.link.read_u32(Regs.DWT_CYCCNT)
            if cycles_now < cycles_last:
                cycles_overflow += 1
            cycles_last = cycles_now

            time.sleep(0.2)

        n_cycles = self.link.read_u32(Regs.DWT_CYCCNT) | (cycles_overflow << 32)

        cpu_time_scale = 1000
        cpu_time = (n_cycles * cpu_time_scale // self.f_cpu) / cpu_time_scale

        stack_image_r = self.link.read_mem(self.ram_stack_ptr, self.ram_stack_sz)
        stack_used = TargetManager.__get_used_stack(stack_image, stack_image_r)

        return ExecutionResult(n_cycles, cpu_time, stack_used)

    def erase_scratchpad(self):
        self.fill_memory(self.ram_scratchpad_ptr, self.ram_stack_ptr, 0xFF)
