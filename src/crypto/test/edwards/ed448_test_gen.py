import itertools
import sys
from typing import NamedTuple, Optional

from testgen.rfc8032_ref import Edwards448Point
from testgen.utils import random_number, random_bytes, print_buffer, number_to_bytes


B = 448
L = Edwards448Point.stdbase().l()


class LoadTest(NamedTuple):
    encoded: bytes
    point: Optional[Edwards448Point]


def random_point(label: str) -> Edwards448Point:
    base = Edwards448Point.stdbase()
    k = random_number(base.l(), label)
    return base * k


def print_point(out, name: str, p: Edwards448Point):
    xb = (p.x / p.z).tobytes(B)
    yb = (p.y / p.z).tobytes(B)

    out.write('    .x%s = {\n' % name)
    print_buffer(xb, out, '      ')
    out.write('    },\n')
    out.write('    .y%s = {\n' % name)
    print_buffer(yb, out, '      ')
    out.write('    },\n')


def random_load_test() -> LoadTest:
    data = bytearray(random_bytes(57, 'random_load'))
    data[56] = data[56] & 0x80
    data = bytes(data)

    point = Edwards448Point.stdbase().decode(data)
    return LoadTest(data, point)


def run():
    points = [random_point('ed448') for _ in range(20)]
    scalars = [random_number(L, 'ed448_s') for _ in range(10)]

    point_pairs = itertools.product(points, points)
    point_scalars = itertools.product(points, scalars)

    load_tests = itertools.chain(
        map(lambda xx: LoadTest(xx.encode(), xx), points),
        [random_load_test() for _ in range(40)]
    )

    out = sys.stdout
    close_out = False
    if len(sys.argv) > 1:
        out = open(sys.argv[1], 'w', encoding='utf-8')
        close_out = True

    out.write('#include <edwards/ed448_test_data.hpp>\n')

    out.write('\nconst ed448_add_test * const ed448_add_tests[] = {\n')
    for i, (x, y) in enumerate(point_pairs):
        out.write('  /* %03d */ (const ed448_add_test []) {{\n' % i)
        print_point(out, '1', x)
        print_point(out, '2', y)
        print_point(out, 'r', x + y)
        out.write('  }},\n')
    out.write('  nullptr\n};\n')

    out.write('\nconst ed448_mul_test * const ed448_mul_tests[] = {\n')
    for i, (x, k) in enumerate(point_scalars):
        out.write('  /* %03d */ (const ed448_mul_test []) {{\n' % i)
        print_point(out, 'i', x)
        out.write('    .k = {\n')
        print_buffer(number_to_bytes(k, B // 8), out, '      ')
        out.write('    },\n')
        print_point(out, 'r', x * k)
        out.write('  }},\n')
    out.write('  nullptr\n};\n')

    out.write('\nconst ed448_load_test * const ed448_load_tests[] = {\n')
    for i, x in enumerate(load_tests):
        out.write('  /* %03d */ (const ed448_load_test []) {{\n' % i)
        if x.point is not None:
            print_point(out, '', x.point)
        out.write('    .valid = %s,\n' % ('true' if x.point is not None else 'false'))
        out.write('    .b = {\n')
        print_buffer(x.encoded, out, '      ')
        out.write('    }\n')
        out.write('  }},\n')
    out.write('  nullptr\n};\n')

    if close_out:
        out.close()


if __name__ == '__main__':
    run()
