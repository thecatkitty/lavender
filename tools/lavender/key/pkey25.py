import re
from typing import Generator

BASE24_ALPHABET = "2346789BCDFGHJKMPQRTVWXY"
GROUP_PATTERN = "[" + BASE24_ALPHABET + "]{5}"
PRODUCT_KEY_PATTERN = GROUP_PATTERN + "(-" + GROUP_PATTERN + "){4}"
PKEY25XOR2B_MASK = [0x00000000000000,
                    0x55555555555555,
                    0xAAAAAAAAAAAAAA,
                    0xFFFFFFFFFFFFFF]


def base24_decode(string: str) -> int:
    ret = 0
    for c in string:
        ret *= 24
        ret += BASE24_ALPHABET.find(c)

    return ret


def base24_encode(number: int) -> str:
    ret = str()
    while number != 0:
        ret = BASE24_ALPHABET[number % 24] + ret
        number //= 24

    return ret


def _format(left: int, right: int) -> str:
    head = base24_encode(left).rjust(13, BASE24_ALPHABET[0])
    tail = base24_encode(right).rjust(12, BASE24_ALPHABET[0])
    full = head + tail
    if len(full) > 25:
        return None
    return f"{full[:5]}-{full[5:10]}-{full[10:15]}-{full[15:20]}-{full[20:]}"


class PKey25(object):
    def __init__(self, key: bytes):
        if len(key) not in [7, 8, 14]:
            raise ValueError("Key has a wrong length")

        self.key = bytes(key)

    def encode(self, parameter: str = None) -> Generator[list[str], None, None]:
        if parameter is None and len(self.key) == 7:
            raise ValueError("Parameter required")

        if len(self.key) == 7 and len(parameter) == 16:
            # XOR12
            data = int(parameter, 16).to_bytes(8, "big")
            left_part = int.from_bytes(data, "big")
            encoded_key = bytes(k ^ d for k, d in zip(self.key, data[1:]))
            right_part = int.from_bytes(encoded_key, "big")
            pkey = _format(left_part, right_part)
            if pkey is not None:
                yield [pkey]
            return

        if len(self.key) in [7, 14]:
            # XOR2B
            k1 = int.from_bytes(self.key[:7], "big")
            k2 = int.from_bytes(self.key[7:], "big")

            ordinals = slice(0, len(PKEY25XOR2B_MASK))
            if parameter is not None:
                ordinals = slice(int(parameter), int(parameter) + 1)

            for x in PKEY25XOR2B_MASK[ordinals]:
                k1x = k1 ^ x
                k2x = k2 ^ x
                left_part = ((x & 3) << 57) | ((k2x & (1 << 55)) << 1) | k1x
                right_part = k2x & ((1 << 55) - 1)
                pkey = _format(left_part, right_part)
                if pkey is not None:
                    yield [pkey]

    @staticmethod
    def decode(fkey: list[str]) -> "PKey25":
        pkey, length = fkey[0], int(fkey[1])
        m = re.match(PRODUCT_KEY_PATTERN, pkey, re.I)
        if m is None:
            raise ValueError("Product key has a wrong format")
        pkey = pkey.replace("-", "")

        left_part = base24_decode(pkey[:13])
        right_part = base24_decode(pkey[13:])

        if length == 56:
            encoded_key = right_part.to_bytes(7, "big")
            data = left_part.to_bytes(8, "big")
            return PKey25(bytes(k ^ d for k, d in zip(encoded_key, data[1:])))

        if length == 112:
            x = PKEY25XOR2B_MASK[(left_part >> 57) & 3]
            k1 = ((left_part & ((1 << 56) - 1)) ^ x).to_bytes(7, "big")
            k2 = ((right_part | ((left_part >> 1) & (1 << 55)))
                  ^ x).to_bytes(7, "big")
            return PKey25(k1 + k2)
