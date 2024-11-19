from lavender.key.pkey25 import from_base24, to_base24
from lavender.math import parity


def to_access_code(cid: int, uid: int) -> str:
    xcid = cid ^ ((uid >> 12) & 0xFFFFFFFF)

    # 59 bits
    left = ((uid >> 38) << 16) | (xcid >> 16)
    left = (left << 1) | parity(left)
    # 55 bits
    right = ((uid & 0x3FFFFFFFFF) << 16) | (xcid & 0xFFFF)
    right = (right << 1) | parity(right)

    head = to_base24(left, 13)
    tail = to_base24(right, 12)
    full = head + tail
    return f"{full[:5]}-{full[5:10]}-{full[10:15]}-{full[15:20]}-{full[20:]}"


def from_access_code(ac: str) -> tuple[int, int]:
    ac_clean = ac.replace("-", "")
    head, tail = ac_clean[:13], ac_clean[13:]

    left = from_base24(head)
    right = from_base24(tail)
    if (1 == parity(left)) or (1 == parity(right)):
        raise ValueError("Access code has wrong parity")
    left >>= 1
    right >>= 1

    uid = ((left >> 16) << 38) | ((right >> 16) & 0x3FFFFFFFFF)
    xcid = ((left & 0xFFFF) << 16) | (right & 0xFFFF)
    cid = xcid ^ ((uid >> 12) & 0xFFFFFFFF)
    return cid, uid
