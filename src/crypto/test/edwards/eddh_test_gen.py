import sys
import itertools
from typing import Tuple, Optional, NamedTuple, Callable

from testgen.utils import random_bytes, print_buffer
from testgen.rfc7748_ref import X448, X25519, CurveDefn


class PublicTest(NamedTuple):
    priv: bytes
    pub: bytes


class ComputeTest(NamedTuple):
    priv: bytes
    pub: bytes
    secret: bytes


def generate_public_test(curve: CurveDefn, priv: Optional[int] = None) -> PublicTest:
    if priv is None:
        priv = random_bytes(curve.length, 'public_test_' + curve.name)
    else:
        priv = priv.to_bytes(curve.length, 'little')

    priv_s = curve.scalar_fn(priv)
    pub = curve.compute_fn(priv_s, None).to_bytes(curve.length, 'little')
    return PublicTest(priv, pub)


def generate_compute_test(curve: CurveDefn) -> ComputeTest:
    priv = random_bytes(curve.length, 'compute_test_' + curve.name)
    pub_s = random_bytes(curve.length, 'compute_test_pub_' + curve.name)
    pub_s = curve.compute_fn(curve.scalar_fn(pub_s), None)

    pub = pub_s.to_bytes(curve.length, 'little')
    secret = curve.compute_fn(curve.scalar_fn(priv), pub_s).to_bytes(curve.length, 'little')

    return ComputeTest(priv, pub, secret)


def generate_tests(out, curve: CurveDefn):
    public_tests = itertools.chain(
        [generate_public_test(curve, 0)],
        [generate_public_test(curve) for _ in range(40)]
    )

    compute_tests = [generate_compute_test(curve) for _ in range(40)]

    out.write('\nconst eddh_public_test * const %s_public_tests[] = {\n' % curve.name)
    for i, t in enumerate(public_tests):
        out.write('  /* %03d */ (const eddh_public_test []) {{\n' % i)
        out.write('    .prv = {\n')
        print_buffer(t.priv, out, '      ')
        out.write('    },\n')
        out.write('    .pub = {\n')
        print_buffer(t.pub, out, '      ')
        out.write('    }\n')
        out.write('  }},\n')
    out.write('  nullptr\n};\n')

    out.write('\nconst eddh_compute_test * const %s_compute_tests[] = {\n' % curve.name)
    for i, t in enumerate(compute_tests):
        out.write('  /* %03d */ (const eddh_compute_test []) {{\n' % i)
        out.write('    .prv = {\n')
        print_buffer(t.priv, out, '      ')
        out.write('    },\n')
        out.write('    .pub = {\n')
        print_buffer(t.pub, out, '      ')
        out.write('    },\n')
        out.write('    .sec = {\n')
        print_buffer(t.secret, out, '      ')
        out.write('    },\n')
        out.write('  }},\n')
    out.write('  nullptr\n};\n')


def run():
    out = sys.stdout
    close_out = False

    if len(sys.argv) > 1:
        out = open(sys.argv[1], 'w', encoding='utf8')
        close_out = True

    out.write('#include <edwards/eddh_test_data.hpp>\n')

    generate_tests(out, X25519)
    generate_tests(out, X448)

    if close_out:
        out.close()


if __name__ == '__main__':
    run()
