import sys
from typing import NamedTuple, Union, Callable
import hashlib

from testgen.rfc8032_ref import PureEdDSA, pEd25519, pEd448, pEd25519ctx, ed448ph_prehash
from testgen.utils import random_bytes, random_number, bytes_to_c, print_buffer


class EdDSAScheme(NamedTuple):
    pure: PureEdDSA
    pure_ph: PureEdDSA
    prehash: Callable[[bytes], bytes]


class SignTest(NamedTuple):
    key: bytes
    hash: bool
    data: bytes
    sig: bytes


class VerifyTest(NamedTuple):
    key: bytes
    hash: bool
    data: bytes
    sig: bytes
    valid: bool


def to_public_key(scheme: PureEdDSA, k: bytes) -> bytes:
    return scheme.keygen(k)[1]


def generate_pubkey_test(scheme: PureEdDSA) -> bytes:
    key_len = int(scheme.b // 8)
    return random_bytes(key_len, f'pubkey_priv_{key_len}')


def generate_sign_test(scheme: EdDSAScheme) -> SignTest:
    key_len = int(scheme.pure.b // 8)
    key = random_bytes(key_len, f'sign_priv_{key_len}')
    pubkey = to_public_key(scheme.pure, key)

    is_hash = random_number(2, 'sign_hash') != 0
    data_len = random_number(256, 'sign_len')
    raw_data = random_bytes(data_len, 'sign_data')

    if is_hash:
        data = scheme.prehash(raw_data)
        sig = scheme.pure_ph.sign(key, pubkey, data, b'', True)
    else:
        data = raw_data
        sig = scheme.pure.sign(key, pubkey, data, None, False)

    return SignTest(key, is_hash, data, sig)


def generate_verify_test(scheme: EdDSAScheme) -> VerifyTest:
    s = generate_sign_test(scheme)
    valid = random_number(2, 'verify_valid') != 0
    sig = s.sig if valid else random_bytes(64, 'verify_fake_sig')
    return VerifyTest(to_public_key(scheme.pure, s.key), s.hash, s.data, sig, valid)


def write_test(out, scheme: PureEdDSA, t: Union[SignTest, VerifyTest], last: bool):
    if isinstance(t, SignTest):
        key = t.key + to_public_key(scheme, t.key)
    else:
        key = t.key

    out.write('    .key = {\n')
    print_buffer(key, out, '      ')
    out.write('    },\n')

    out.write('    .len = %s,\n' % ('MSG_LEN_PREHASH' if t.hash else str(len(t.data))))
    out.write('    .msg = {\n')
    print_buffer(t.data, out, '      ')
    out.write('    },\n')
    out.write('    .sig = {\n')
    print_buffer(t.sig, out, '      ')
    out.write('    }%s\n' % ('' if last else ','))


def generate_eddsa_tests(out, scheme: EdDSAScheme, prefix: str):
    pub_key_samples = [generate_pubkey_test(scheme.pure) for _ in range(20)]
    sign_samples = [generate_sign_test(scheme) for _ in range(40)]
    verify_samples = [generate_verify_test(scheme) for _ in range(40)]

    out.write('\nconst eddsa_public_key_test * const %s_public_key_tests[] = {\n' % prefix)
    for i, t in enumerate(pub_key_samples):
        out.write('  /* %03d */ (const eddsa_public_key_test []) {{\n' % i)
        out.write('    .x = {\n')
        print_buffer(t, out, '      ')
        out.write('    },\n')
        out.write('    .y = {\n')
        print_buffer(to_public_key(scheme.pure, t), out, '      ')
        out.write('    }\n')
        out.write('  }},\n')
    out.write('  nullptr\n};\n')

    out.write('\nconst eddsa_sign_test * const %s_sign_tests[] = {\n' % prefix)
    for i, t in enumerate(sign_samples):
        out.write('  /* %03d */ (const eddsa_sign_test []) {{\n' % i)
        write_test(out, scheme.pure, t, True)
        out.write('  }},\n')
    out.write('  nullptr\n};\n')

    out.write('\nconst eddsa_verify_test * const %s_verify_tests[] = {\n' % prefix)
    for i, t in enumerate(verify_samples):
        out.write('  /* %03d */ (const eddsa_verify_test []) {{\n' % i)
        write_test(out, scheme.pure, t, False)
        out.write('    .valid = %s\n' % str(t.valid).lower())
        out.write('  }},\n')
    out.write('  nullptr\n};\n')


def run():
    out = sys.stdout
    close_out = False
    if len(sys.argv) > 1:
        out = open(sys.argv[1], 'w', encoding='utf-8')
        close_out = True

    out.write('#include <edwards/eddsa_test_data.hpp>\n')

    ed25519 = EdDSAScheme(pEd25519, pEd25519ctx, lambda x: hashlib.sha512(x).digest())
    generate_eddsa_tests(out, ed25519, 'eddsa25519')

    ed448 = EdDSAScheme(pEd448, pEd448, lambda x: ed448ph_prehash(x, b''))
    generate_eddsa_tests(out, ed448, 'eddsa448')

    if close_out:
        out.close()


if __name__ == '__main__':
    run()