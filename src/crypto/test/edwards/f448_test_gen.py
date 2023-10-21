import sys

from testgen.utils import random_number, print_buffer, number_to_bytes
import itertools


B = 448
Q = 2**448 - 2**224 - 1
P = 2**448


def print_number(out, name: str, value: int):
    out.write('    // %0112X\n' % value)
    out.write('    .%s = {\n' % name)
    print_buffer(number_to_bytes(value, length=56), out, prefix='      ')
    out.write('    },\n')


# Modular reduction which could produce both Q and 2Q results:
def f448_reduce(x: int) -> int:
    x = x % (2 * Q)
    if x >= P:
        x -= Q

    return x


def run():
    special_values = [0, 1, 2, Q - 2, Q - 1, Q, Q + 1, Q + 2, P - 2, P - 1]
    random_values = [random_number(1 << 448, 'f448') for _ in range(80)]
    random_squares = [f448_reduce(x * x) for x in map(lambda _: random_number(Q, 'f448_sq'), range(40))]

    load_special = [-1, -2, 0, 1, 2]
    load_random = [random_number(1 << 32, 'load') - (1 << 31) for _ in range(20)]

    load_samples = itertools.chain(load_special, load_random)
    unary_samples = itertools.chain(special_values, random_values, random_squares)
    binary_samples = filter(lambda xx: xx[0] <= xx[1], itertools.chain(
        itertools.product(special_values, special_values),
        itertools.product(special_values, random_values),
        itertools.product(random_values, random_values)
    ))

    out = sys.stdout
    close_out = False

    if len(sys.argv) > 1:
        out = open(sys.argv[1], 'w', encoding='utf-8')
        close_out = True

    out.write('#include <edwards/f448_test_data.hpp>\n')

    out.write('\nconst f448_load_test * const f448_load_tests[] = {\n')
    for i, x in enumerate(load_samples):
        out.write('  /* %03d */ (const f448_load_test []) {{\n' % i)
        out.write('    .v = %d,\n' % x)
        print_number(out, 'x', x % Q)
        out.write('  }},\n')
    out.write('  nullptr\n};\n')

    out.write('\nconst f448_unary_test * const f448_unary_tests[] = {\n')
    for i, x in enumerate(unary_samples):
        out.write('  /* %03d */ (const f448_unary_test []) {{\n' % i)
        print_number(out, 'x', x)
        print_number(out, 'z', x % Q)
        print_number(out, 'n', (Q - x) % Q)
        print_number(out, 'i', pow(x, Q - 2, Q))
        print_number(out, 'q', pow(x, (Q - 3) // 4, Q))
        out.write('  }},\n')
    out.write('  nullptr\n};\n')

    out.write('\nconst f448_binary_test * const f448_binary_tests[] = {\n')
    for i, (x, y) in enumerate(binary_samples):
        out.write('  /* %03d */ (const f448_binary_test []) {{\n' % i)
        print_number(out, 'x', x)
        print_number(out, 'y', y)
        print_number(out, 's', (x + y) % Q)
        print_number(out, 'd', (x - y) % Q)
        print_number(out, 'e', (y - x) % Q)
        print_number(out, 'p', (x * y) % Q)
        out.write('  }},\n')
    out.write('  nullptr\n};\n')

    if close_out:
        out.close()


if __name__ == '__main__':
    run()
