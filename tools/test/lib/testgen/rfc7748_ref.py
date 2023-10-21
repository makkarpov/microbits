from typing import Tuple, NamedTuple, Optional, Callable


def _cswap(cond: int, a: int, b: int) -> Tuple[int, int]:
    if cond == 1:
        return b, a
    else:
        return a, b


def _curve_multiply(u: int, k: int, p: int, bits: int, a24: int) -> int:
    x2 = 1
    z2 = 0
    x3 = u
    z3 = 1
    swap = 0

    for t in range(bits - 1, -1, -1):
        k_t = (k >> t) & 1
        swap ^= k_t

        x2, x3 = _cswap(swap, x2, x3)
        z2, z3 = _cswap(swap, z2, z3)
        swap = k_t

        a = (x2 + z2) % p
        aa = (a * a) % p
        b = (x2 - z2) % p
        bb = (b * b) % p
        e = (aa - bb) % p
        c = (x3 + z3) % p
        d = (x3 - z3) % p
        da = (d * a) % p
        cb = (c * b) % p
        x3 = (da + cb) * (da + cb) % p
        z3 = u * (da - cb) * (da - cb) % p
        x2 = (aa * bb) % p
        z2 = (e * (aa + a24 * e)) % p

    x2, x3 = _cswap(swap, x2, x3)
    z2, z3 = _cswap(swap, z2, z3)
    return (x2 * pow(z2, p - 2, p)) % p


def x25519(k: int, u: Optional[int] = None) -> int:
    return _curve_multiply(u or 9, k, 2 ** 255 - 19, 255, 121665)


def x25519_load_scalar(b: bytes) -> int:
    s = bytearray(b)
    s[0] &= 248
    s[31] &= 127
    s[31] |= 64
    return int.from_bytes(s, 'little')


def x448(k: int, u: Optional[int] = None) -> int:
    return _curve_multiply(u or 5, k, 2 ** 448 - 2 ** 224 - 1, 448, 39081)


def x448_load_scalar(b: bytes) -> int:
    s = bytearray(b)
    s[0] &= 252
    s[55] |= 128
    return int.from_bytes(s, 'little')


class CurveDefn(NamedTuple):
    name: str
    length: int
    order: int
    compute_fn: Callable[[int, Optional[int]], int]
    scalar_fn: Callable[[bytes], int]


X25519 = CurveDefn(
    name='x25519',
    length=32,
    order=2**252 + 0x14def9dea2f79cd65812631a5cf5d3ed,
    compute_fn=x25519,
    scalar_fn=x25519_load_scalar
)

X448 = CurveDefn(
    name='x448',
    length=56,
    order=2**446 - 0x8335dc163bb124b65129c96fde933d8d723a70aadc873d6d54a7bb0d,
    compute_fn=x448,
    scalar_fn=x448_load_scalar
)
