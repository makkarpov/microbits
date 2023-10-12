from typing import NamedTuple
import sys

from testgen.utils import random_number, number_to_c, print_buffer


class LoadTest(NamedTuple):
    n: int
    n_len: int
    m: int


class BinaryTest(NamedTuple):
    a: int
    b: int
    m: int


def gen_load_test() -> LoadTest:
    n_len = 32 + random_number(64, 'load_len')
    n = random_number(1 << (8 * n_len), 'load_v')
    m = random_number(1 << 255, 'load_mod')
    return LoadTest(n, n_len, m)


def gen_binary_test() -> BinaryTest:
    m = random_number(1 << 255, 'bin_mod')
    a = random_number(m, 'bin_arg')
    b = random_number(m, 'bin_arg')
    return BinaryTest(a, b, m)


def run():
    load_tests = [gen_load_test() for _ in range(100)]
    binary_tests = [gen_binary_test() for _ in range(100)]

    out = sys.stdout
    close_out = False
    if len(sys.argv) > 1:
        out = open(sys.argv[1], 'w', encoding='utf-8')
        close_out = True

    out.write('#include <c25519/fprime_test_data.hpp>\n')

    out.write('\nconst fp_load_test * const fp_load_tests[] = {\n')
    for i, t in enumerate(load_tests):
        out.write('  /* %03d */ (const fp_load_test []) {{\n' % i)
        out.write('    // 0x%X\n' % t.n)
        out.write('    .il = %d,\n' % t.n_len)
        out.write('    .i = {\n')
        print_buffer(t.n.to_bytes(t.n_len, 'little'), out, '      ')
        out.write('    },\n')
        out.write('    // 0x%064X\n' % t.m)
        out.write('    .m = %s,\n' % number_to_c(t.m))
        out.write('    // 0x%064X\n' % (t.n % t.m))
        out.write('    .r = %s\n' % number_to_c(t.n % t.m))
        out.write('  }},\n')
    out.write('  nullptr\n};\n')

    out.write('\nconst fp_binary_test * const fp_binary_tests[] = {\n')
    for i, t in enumerate(binary_tests):
        out.write('  /* %03d */ (const fp_binary_test []) {{\n' % i)
        out.write('    // 0x%064X\n' % t.a)
        out.write('    .a = %s,\n' % number_to_c(t.a))
        out.write('    // 0x%064X\n' % t.b)
        out.write('    .b = %s,\n' % number_to_c(t.b))
        out.write('    // 0x%064X\n' % t.m)
        out.write('    .m = %s,\n' % number_to_c(t.m))
        out.write('    .s = %s,\n' % number_to_c((t.a + t.b) % t.m))
        out.write('    .p = %s\n' % number_to_c((t.a * t.b) % t.m))
        out.write('  }},\n')
    out.write('  nullptr\n};\n')

    if close_out:
        out.close()


if __name__ == '__main__':
    run()
