import sys
import itertools
from typing import Tuple, Optional
from testgen.utils import random_number, number_to_c, bytes_to_c


# Copied from reference implementation at https://ed25519.cr.yp.to/python/ed25519.py

kQ = 2 ** 255 - 19
kD = (-121665 * pow(121666, kQ - 2, kQ)) % kQ
kI = pow(2, (kQ - 1) // 4, kQ)
kL = 2**252 + 27742317777372353535851937790883648493


def inv(x: int) -> int:
    return pow(x, kQ - 2, kQ)


def edwards(x1: int, y1: int, x2: int, y2: int) -> Tuple[int, int]:
    x3 = (x1 * y2 + x2 * y1) * inv(1 + kD * x1 * x2 * y1 * y2)
    y3 = (y1 * y2 + x1 * x2) * inv(1 - kD * x1 * x2 * y1 * y2)
    return x3 % kQ, y3 % kQ


def scalar_mult(x1: int, y1: int, e: int) -> Tuple[int, int]:
    if e == 0:
        return 0, 1

    (qx, qy) = scalar_mult(x1, y1, e >> 1)
    (qx, qy) = edwards(qx, qy, qx, qy)

    if (e & 1) != 0:
        (qx, qy) = edwards(qx, qy, x1, y1)

    return qx, qy


def x_recover(y: int) -> int:
    xx = (y*y - 1) * inv(kD * y * y + 1)
    x = pow(xx, (kQ + 3) // 8, kQ)

    if (x*x - xx) % kQ != 0:
        x = (x * kI) % kQ

    if x % 2 != 0:
        x = kQ - x

    return x


By = (4 * inv(5)) % kQ
Bx = x_recover(By)


def is_on_curve(x, y) -> bool:
    return (-x * x + y * y - 1 - kD * x * x * y * y) % kQ == 0


def decode_point(s: bytes) -> Optional[Tuple[int, int]]:
    y = int.from_bytes(s, byteorder='little')
    x_p = (y >> 255) & 1
    y = y & ((1 << 255) - 1)

    x = x_recover(y)
    if x & 1 != x_p:
        x = kQ - x

    if not is_on_curve(x, y):
        return None

    return x, y


def encode_point(x: int, y: int) -> bytes:
    s = y | ((x & 1) << 255)
    return s.to_bytes(32, 'little')


def random_point() -> Tuple[int, int]:
    s = random_number(kL)
    return scalar_mult(Bx, By, s)


def run():
    out = sys.stdout
    close_out = False

    if len(sys.argv) > 1:
        out = open(sys.argv[1], 'w', encoding='utf-8')
        close_out = True

    random_points = [random_point() for _ in range(10)]
    random_numbers = [random_number(kQ) for _ in range(10)]

    add_samples = itertools.product(random_points, random_points)
    mul_samples = map(
        lambda xx: (xx, random_number(kL)),
        random_points
    )

    load_samples = itertools.chain(
        map(lambda xx: encode_point(xx[0], xx[1]), random_points),
        map(lambda xx: xx.to_bytes(32, 'little'), random_numbers)
    )

    out.write('#include <c25519/ed25519_test_data.hpp>\n')

    out.write('\nconst ed25519_add_test * const ed25519_add_tests[] = {\n')
    for i, (p1, p2) in enumerate(add_samples):
        (rx, ry) = edwards(p1[0], p1[1], p2[0], p2[1])
        out.write('  /* %03d */ (const ed25519_add_test []) {{\n' % i)
        out.write('     // 0x%064X, 0x%064X\n' % p1)
        out.write('    .x1 = %s,\n' % number_to_c(p1[0]))
        out.write('    .y1 = %s,\n' % number_to_c(p1[1]))
        out.write('     // 0x%064X, 0x%064X\n' % p2)
        out.write('    .x2 = %s,\n' % number_to_c(p2[0]))
        out.write('    .y2 = %s,\n' % number_to_c(p2[1]))
        out.write('     // 0x%064X, 0x%064X\n' % (rx, ry))
        out.write('    .xr = %s,\n' % number_to_c(rx))
        out.write('    .yr = %s,\n' % number_to_c(ry))
        out.write('  }},\n')
    out.write('  nullptr\n};\n')

    out.write('\nconst ed25519_mul_test * const ed25519_mul_tests[] = {\n')
    for i, (p, k) in enumerate(mul_samples):
        (rx, ry) = scalar_mult(p[0], p[1], k)
        out.write('  /* %03d */ (const ed25519_mul_test []) {{\n' % i)
        out.write('    .x1 = %s,\n' % number_to_c(p[0]))
        out.write('    .y1 = %s,\n' % number_to_c(p[1]))
        out.write('    .k  = %s,\n' % number_to_c(k))
        out.write('    .xr = %s,\n' % number_to_c(rx))
        out.write('    .yr = %s,\n' % number_to_c(ry))
        out.write('  }},\n')
    out.write('  nullptr\n};\n')

    out.write('\nconst ed25519_load_test * const ed25519_load_tests[] = {\n')
    for i, b in enumerate(load_samples):
        r = decode_point(b)
        out.write('  /* %03d */ (const ed25519_load_test []) {{\n' % i)
        out.write('    .b  = %s,\n' % bytes_to_c(b))
        if r is not None:
            out.write('    .valid = true,\n')
            out.write('    .xr = %s,\n' % number_to_c(r[0]))
            out.write('    .yr = %s,\n' % number_to_c(r[1]))
        else:
            out.write('    .valid = false\n')
        out.write('  }},\n')
    out.write(' nullptr\n};\n')

    if close_out:
        out.close()


if __name__ == '__main__':
    run()
