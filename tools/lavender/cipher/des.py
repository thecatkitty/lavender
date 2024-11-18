from io import BytesIO
from Crypto.Cipher import DES, DES3
from Crypto.Util.Padding import pad, unpad


def _get_cipher(key: bytes) -> object:
    if 8 == len(key):
        return DES.new(key, DES.MODE_CBC)  # nosec

    if 16 == len(key):
        return DES3.new(key, DES3.MODE_CBC)


def _parity(n: int) -> int:
    result = 1
    while n:
        result ^= n & 1
        n >>= 1
    return result


def _expand(key: bytes) -> bytes:
    key_size = len(key) // 2
    key_num = int.from_bytes(key, "big")
    key_parts = [((key_num >> (i * 7)) & 0x7F) <<
                 1 for i in reversed(range((key_size + 1) * 2))]
    return bytes(b | _parity(b) for b in key_parts)


def decrypt(input: BytesIO, output: BytesIO, key: bytes) -> None:
    xkey = _expand(key)
    cipher = _get_cipher(xkey)

    ivct = input.read()
    data = unpad(cipher.decrypt(ivct)[8:], DES.block_size)
    output.write(data)


def encrypt(input: BytesIO, output: BytesIO, key: bytes) -> str:
    xkey = _expand(key)
    cipher = _get_cipher(xkey)

    data = input.read()
    ct = cipher.encrypt(pad(data, DES.block_size))
    output.write(cipher.iv)
    output.write(ct)
    return "".join(f"{b:02X}" for b in xkey)
