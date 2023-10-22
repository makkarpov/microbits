import sys
from typing import NamedTuple
from Crypto.Cipher import AES, ChaCha20

from testgen.utils import print_buffer, random_bytes


class BlockTest(NamedTuple):
    enc: bool
    key: bytes
    input: bytes
    output: bytes


def generate_block_test(name: str, cipher, key_length: int) -> BlockTest:
    key = random_bytes(key_length, f'block_cipher_k_{name}_{key_length}')
    input_d = random_bytes(cipher.block_size, f'block_cipher_i_{name}')
    output_d = cipher.new(key, cipher.MODE_ECB).encrypt(input_d)
    return BlockTest(True, key, input_d, output_d)


def run():
    if len(sys.argv) == 1:
        raise RuntimeError('cipher name is missing')

    cipher_name = sys.argv[1]
    match cipher_name:
        case 'aes': cipher = AES
        case _: raise RuntimeError('unknown cipher name: %s' % cipher_name)

    block_tests = [generate_block_test(cipher_name, cipher, k) for k in cipher.key_size for _ in range(100)]

    out = sys.stdout
    close_out = False
    if len(sys.argv) > 2:
        out = open(sys.argv[2], 'w', encoding='utf8')
        close_out = True

    out.write('#include <cipher/block_test_data.hpp>\n')

    out.write('\nconst cipher_block_test * const %s_block_tests[] = {\n' % cipher_name.lower())
    for i, t in enumerate(block_tests):
        out.write('  /* %03d */ (const cipher_block_test []) {{\n' % i)
        out.write('    .enc = %s,\n' % str(t.enc).lower())
        out.write('    .kl = %d,\n' % len(t.key))
        out.write('    .k = {\n')
        print_buffer(t.key, out, '      ')
        out.write('    },\n')
        out.write('    .i = {\n')
        print_buffer(t.input, out, '      ')
        out.write('    },\n')
        out.write('    .o = {\n')
        print_buffer(t.output, out, '      ')
        out.write('    },\n')
        out.write('  }},\n')
    out.write('  nullptr\n};\n')

    if close_out:
        out.close()


if __name__ == '__main__':
    run()
