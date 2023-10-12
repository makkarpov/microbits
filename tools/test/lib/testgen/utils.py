from typing import Literal


def random_bytes(length: int, label: str) -> bytes:
    """
    Deterministic random bytes generator

    :param length: Number of bytes to produce
    :param label:  Unique identifier of produced sequence
    :return:       Random byte array
    """
    if not hasattr(random_bytes, 'counters'):
        random_bytes.counters = {}

    counter = random_bytes.counters.get(label) or 0
    random_bytes.counters[label] = counter + 1

    r = b''
    i = 0
    label = label.encode('utf-8')

    from hashlib import sha512
    while len(r) < length:
        r += sha512(label + counter.to_bytes(4, 'big') + i.to_bytes(4, 'big')).digest()
        i += 1

    return r[:length]


def random_number(limit: int, label: str = '') -> int:
    """
    Deterministic random number generator

    :param limit: Maximum value (exclusive) for returned numbers
    :param label: Unique identifier of produced sequence
    :return:      Random number in bounds [0, limit)
    """

    n_bytes = (2 * int.bit_length(limit) + 7) // 8
    r_label = 'random_number:' + (label if len(label) > 0 else str(limit))
    return int.from_bytes(random_bytes(n_bytes, r_label), 'big') % limit


def bytes_to_c(b: bytes) -> str:
    """ Convert a byte array into C representation """
    return '{' + ','.join(['0x%02X' % v for v in b]) + '}'


def number_to_bytes(i: int, length: int = 0, order: Literal['big', 'little'] = 'little') -> bytes:
    """ Convert an integer into byte array representation """

    if length == 0:
        length = max(1, (int.bit_length(i) + 7) // 8)

    return i.to_bytes(length, order)


def number_to_c(i: int, length: int = 0) -> str:
    """ Convert an integer into C byte array representation """
    return bytes_to_c(number_to_bytes(i, length))


def print_buffer(data: bytes, out, prefix: str = '  ', bytes_per_line: int = 0):
    if bytes_per_line == 0:
        bytes_per_line = (120 - len(prefix)) // 5

    ofs = 0
    while ofs < len(data):
        s_len = min(len(data) - ofs, bytes_per_line)
        out.write(prefix + ','.join(['0x%02X' % b for b in data[ofs:ofs+s_len]]))
        ofs += s_len

        if ofs != len(data):
            out.write(',')

        out.write('\n')
