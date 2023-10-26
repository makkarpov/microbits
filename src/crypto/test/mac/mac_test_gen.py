import sys
from typing import NamedTuple, Callable, Any, Optional, List
from Crypto.Hash import KMAC128, KMAC256, HMAC, SHA256, SHA512

from testgen.utils import random_bytes, print_buffer, random_number


class MACDefinition(NamedTuple):
    ctor: Callable[[bytes, int], Any]   # (key, mac_len) -> MAC
    mac_length: Optional[int]
    key_length: int


class MACTest(NamedTuple):
    data_len: int
    key: bytes
    mac: bytes


def generate_test(mac_name: str, mac: MACDefinition, data: bytes) -> MACTest:
    key_len = mac.key_length + random_number(256 - mac.key_length, f'mac_klen_{mac_name}')
    key = random_bytes(key_len, f'mac_key_{mac_name}')

    mac_len = mac.mac_length
    if mac.mac_length is None:
        mac_len = 8 + random_number(120, f'mac_len_{mac_name}')

    mac_inst = mac.ctor(key, mac_len)
    mac_inst.update(data)
    return MACTest(len(data), key, mac_inst.digest())


def generate_tests(mac_name: str, mac: MACDefinition, data: bytes) -> List[MACTest]:
    dl = 0
    ret = []

    while dl < len(data):
        if dl < 256:
            dl += 1
        elif dl < 512:
            dl += 11
        else:
            dl += 97

        dl = min(dl, len(data))
        ret.append(generate_test(mac_name, mac, data[:dl]))

    return ret


def run():
    if len(sys.argv) < 2:
        raise RuntimeError('MAC name is not specified')

    mac_name = sys.argv[1]
    match mac_name:
        case 'sha256': mac_defn = MACDefinition(lambda k, _: HMAC.new(k, digestmod=SHA256), SHA256.digest_size, 1)
        case 'sha512': mac_defn = MACDefinition(lambda k, _: HMAC.new(k, digestmod=SHA512), SHA512.digest_size, 1)
        case 'kmac128': mac_defn = MACDefinition(lambda k, s: KMAC128.new(key=k, mac_len=s), None, 16)
        case 'kmac256': mac_defn = MACDefinition(lambda k, s: KMAC256.new(key=k, mac_len=s), None, 32)
        case _: raise RuntimeError('Unknown MAC name: %s' % mac_name)

    data_buffer = random_bytes(8192, f'mac_test_data_{mac_name}')
    tests = generate_tests(mac_name, mac_defn, data_buffer)

    out = sys.stdout
    close_out = False
    if len(sys.argv) > 2:
        out = open(sys.argv[2], 'w', encoding='utf8')
        close_out = True

    out.write('#include <mac/mac_test_data.hpp>\n')

    data_name = f'mac_data_{mac_name}'
    out.write('\nstatic const uint8_t %s[] = {\n' % data_name)
    print_buffer(data_buffer, out, '  ')
    out.write('};\n')

    out.write('\nconst mac_test * const %s_mac_tests[] = {\n' % mac_name)
    for i, t in enumerate(tests):
        out.write('  /* %03d */ (const mac_test []) {{\n' % i)
        out.write('    .data = %s,\n' % data_name)
        out.write('    .len  = %d,\n' % t.data_len)
        out.write('    .kl = %d,\n' % len(t.key))
        out.write('    .k = {\n')
        print_buffer(t.key, out, '      ')
        out.write('    },\n')
        out.write('    .ml = %d,\n' % len(t.mac))
        out.write('    .m = {\n')
        print_buffer(t.mac, out, '      ')
        out.write('    }\n')
        out.write('  }},\n')
    out.write('  nullptr\n};\n')

    if close_out:
        out.close()


if __name__ == '__main__':
    run()
