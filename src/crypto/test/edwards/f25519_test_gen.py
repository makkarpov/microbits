import sys
import itertools
from testgen.utils import random_number, number_to_c


def random_sq(q: int) -> int:
    """ Generate deterministic random number which is a square mod q """
    x = random_number(q)
    return (x * x) % q


def sqrt_mod_q(x: int, q: int) -> int:
    """ Compute a square root of x mod q """
    u = pow(x, (q + 3) // 8, q)

    if (u * u - x) % q != 0:
        ii = pow(2, (q - 1) // 4, q)
        u = (u * ii) % q

    if (u * u - x) % q != 0:
        u = 0

    if u % 2 != 0:
        u = q - u

    return u


def run():
    q = 2 ** 255 - 19  # curve25519 number field order
    b = 2 ** 256 - 1   # maximum representable value of uint256_t

    special_values = [0, 1, 2, (q - 1) // 2, q - 2, q - 1, q, q + 1, q + 2, 2 * q - 2, 2 * q - 1]
    special_denorm = [2 * q, 2 * q + 1, 2 * q + 2, b - 2, b - 1, b]

    random_values = [random_number(2 * q) for _ in range(40)]
    random_squares = [random_sq(2 * q) for _ in range(40)]
    random_denorm = [random_number(b) for _ in range(40)]

    norm_samples = itertools.chain(special_values, random_values, special_denorm, random_denorm)
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

    out.write('#include <edwards/f25519_test_data.hpp>\n')

    out.write('\nconst f25519_norm_test * const f25519_norm_tests[] = {\n')
    for i, x in enumerate(norm_samples):
        out.write('  // (%03d) 0x%064X -> 0x%064X\n' % (i, x, x % q))
        out.write('  (const f25519_norm_test []) {{\n')
        out.write('    .x = ' + number_to_c(x) + ',\n')
        out.write('    .y = ' + number_to_c(x % q) + '\n')
        out.write('  }},\n')
    out.write('  nullptr\n};\n')

    out.write('\nconst f25519_unary_test * const f25519_unary_tests[] = {\n')
    for i, x in enumerate(unary_samples):
        r = sqrt_mod_q(x % q, q)
        out.write('  // (%03d) 0x%064X\n' % (i, x))
        out.write('  (const f25519_unary_test []) {{\n')
        out.write('    .x = ' + number_to_c(x) + ',\n')
        out.write('    .n = ' + number_to_c(q - x % q) + ',\n')
        out.write('    .i = ' + number_to_c(pow(x, q - 2, q)) + ',\n')
        if r != 0:
            out.write('    .re = true,\n')
            out.write('    .r = ' + number_to_c(r) + '\n')
        else:
            out.write('    .re = false,\n')
            out.write('    .r = {}\n')

        out.write('  }},\n')
    out.write('  nullptr\n};\n')

    out.write('\nconst f25519_binary_test * const f25519_binary_tests[] = {\n')
    for i, [x, y] in enumerate(binary_samples):
        out.write('  // (%03d) 0x%064X (?) 0x%064X\n' % (i, x, y))
        out.write('  (const f25519_binary_test []) {{\n')
        out.write('    .x = ' + number_to_c(x) + ',\n')
        out.write('    .y = ' + number_to_c(y) + ',\n')
        out.write('    .s = ' + number_to_c((x + y) % q) + ',\n')
        out.write('    .d = ' + number_to_c((x - y) % q) + ',\n')
        out.write('    .e = ' + number_to_c((y - x) % q) + ',\n')
        out.write('    .p = ' + number_to_c((x * y) % q) + '\n')
        out.write('  }},\n')
    out.write('  nullptr\n};\n')

    if close_out:
        out.close()


if __name__ == '__main__':
    run()
