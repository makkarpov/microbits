import sys
from typing import NamedTuple, Any
from Crypto.Cipher import ChaCha20

from testgen.utils import random_bytes, print_buffer


class CipherDefn(NamedTuple):
    cipher: Any
    nonce_len: int


class StreamTest(NamedTuple):
    key: bytes
    nonce: bytes
    output: bytes


def generate_stream_test(name: str, cipher: CipherDefn) -> StreamTest:
    key = random_bytes(cipher.cipher.key_size, f'stream_key_{name}')
    nonce = random_bytes(cipher.nonce_len, f'stream_nonce_{name}')
    data = cipher.cipher.new(key=key, nonce=nonce).encrypt(b'\x00' * 256)
    return StreamTest(key, nonce, data)


def run():
    if len(sys.argv) < 2:
        raise RuntimeError('cipher name is not set')

    cipher_name = sys.argv[1]
    match cipher_name:
        case 'chacha20': cipher = CipherDefn(ChaCha20, 12)
        case _: raise RuntimeError('unknown cipher name: %s' % cipher_name)

    tests = [generate_stream_test(cipher_name, cipher) for _ in range(100)]

    out = sys.stdout
    close_out = False
    if len(sys.argv) > 2:
        out = open(sys.argv[2], 'w', encoding='utf8')
        close_out = True

    out.write('#include <cipher/stream_test_data.hpp>\n')

    out.write('\nconst cipher_stream_test * const %s_tests[] = {\n' % cipher_name)
    for i, t in enumerate(tests):
        out.write('  /* %03d */ (const cipher_stream_test []) {{\n' % i)
        out.write('    .kl = %d,\n' % len(t.key))
        out.write('    .k = {\n')
        print_buffer(t.key, out, '      ')
        out.write('    },\n')
        out.write('    .nl = %d,\n' % len(t.nonce))
        out.write('    .n = {\n')
        print_buffer(t.nonce, out, '      ')
        out.write('    },\n')
        out.write('    .sl = %d,\n' % len(t.output))
        out.write('    .s = {\n')
        print_buffer(t.output, out, '      ')
        out.write('    },\n')
        out.write('  }},\n')
    out.write('  nullptr\n};\n')

    if close_out:
        out.close()


if __name__ == '__main__':
    run()
