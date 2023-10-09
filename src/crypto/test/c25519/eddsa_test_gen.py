import sys
from typing import NamedTuple, Union
from Crypto.Hash import SHA512
from Crypto.Signature import eddsa
from test_utils import random_bytes, random_number, bytes_to_c, print_buffer


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


def generate_sign_test() -> SignTest:
    key = random_bytes(32, 'sign_priv')
    key_obj = eddsa.import_private_key(key)

    is_hash = random_number(2, 'sign_hash') != 0
    data_len = random_number(256, 'sign_len')
    raw_data = random_bytes(data_len, 'sign_data')

    if is_hash:
        data = SHA512.new(raw_data).digest()
        sig = eddsa.EdDSASigScheme(key_obj, b'').sign(SHA512.new(raw_data))
    else:
        data = raw_data
        sig = eddsa.EdDSASigScheme(key_obj, b'').sign(raw_data)

    return SignTest(key, is_hash, data, sig)


def to_public_key(k: bytes) -> bytes:
    return eddsa.import_private_key(k).public_key().export_key(format='raw')


def generate_verify_test() -> VerifyTest:
    s = generate_sign_test()
    valid = random_number(2, 'verify_valid') != 0
    sig = s.sig if valid else random_bytes(64, 'verify_fake_sig')
    return VerifyTest(to_public_key(s.key), s.hash, s.data, sig, valid)


def write_test(out, t: Union[SignTest, VerifyTest], last: bool):
    out.write('    .key = %s,\n' % bytes_to_c(t.key))
    out.write('    .len = %s,\n' % ('MSG_LEN_ED25519ph' if t.hash else str(len(t.data))))
    out.write('    .msg = {\n')
    print_buffer(t.data, out, '      ')
    out.write('    },\n')
    out.write('    .sig = {\n')
    print_buffer(t.sig, out, '      ')
    out.write('    }%s\n' % ('' if last else ','))


def run():
    pub_key_samples = [random_bytes(32, 'priv_key') for _ in range(20)]
    sign_samples = [generate_sign_test() for _ in range(40)]
    verify_samples = [generate_verify_test() for _ in range(40)]

    out = sys.stdout
    close_out = False
    if len(sys.argv) > 1:
        out = open(sys.argv[1], 'w', encoding='utf-8')
        close_out = True

    out.write('#include <c25519/eddsa_test_data.hpp>\n')

    out.write('\nconst eddsa_public_key_test * const eddsa_public_key_tests[] = {\n')
    for i, t in enumerate(pub_key_samples):
        out.write('  /* %03d */ (const eddsa_public_key_test []) {{\n' % i)
        out.write('    .x = %s,\n' % bytes_to_c(t))
        out.write('    .y = %s\n' % bytes_to_c(to_public_key(t)))
        out.write('  }},\n')
    out.write('  nullptr\n};\n')

    out.write('\nconst eddsa_sign_test * const eddsa_sign_tests[] = {\n')
    for i, t in enumerate(sign_samples):
        out.write('  /* %03d */ (const eddsa_sign_test []) {{\n' % i)
        write_test(out, t, True)
        out.write('  }},\n')
    out.write('  nullptr\n};\n')

    out.write('\nconst eddsa_verify_test * const eddsa_verify_tests[] = {\n')
    for i, t in enumerate(verify_samples):
        out.write('  /* %03d */ (const eddsa_verify_test []) {{\n' % i)
        write_test(out, t, False)
        out.write('    .valid = %s\n' % str(t.valid).lower())
        out.write('  }},\n')
    out.write('  nullptr\n};\n')

    if close_out:
        out.close()


if __name__ == '__main__':
    run()