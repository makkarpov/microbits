import hashlib
from test_utils import print_buffer
import sys


def run():
    if len(sys.argv) < 2:
        print('usage: %s <hash> [output file]' % sys.argv[0], file=sys.stderr)
        sys.exit(1)

    hash_name = sys.argv[1]
    hash_alg = getattr(hashlib, hash_name)

    data_buffer = b''
    for i in range(256):
        data_buffer += hashlib.sha512(hash_name.encode('utf-8') + i.to_bytes(4, 'big')).digest()

    lengths = list(range(256)) + list(range(256, 1024, 32)) + list(range(1024, len(data_buffer), 128))

    out = sys.stdout
    close_out = False
    if len(sys.argv) > 2:
        out = open(sys.argv[2], 'w', encoding='utf-8')
        close_out = True

    out.write('#include <hash/hash_test.hpp>\n')

    out.write('\nconst uint8_t %s_buffer[] = {\n' % hash_name)
    print_buffer(data_buffer, out)
    out.write('};\n')

    out.write('\nconst hash_test_sample * const %s_samples[] = {\n' % hash_name)

    for ll in lengths:
        out.write('  (const hash_test_sample []) {{ \n')
        out.write('    .length = %d,\n' % ll)
        out.write('    .digest = (const uint8_t []) {\n')
        print_buffer(hash_alg(data_buffer[:ll]).digest(), out, prefix='      ', bytes_per_line=16)
        out.write('    }\n')
        out.write('  }},\n')

    out.write('  nullptr\n};\n')

    if close_out:
        out.close()


if __name__ == '__main__':
    run()
