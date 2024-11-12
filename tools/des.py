#!/usr/bin/python3

from argparse import ArgumentParser, FileType
from Crypto.Cipher import DES, DES3
from Crypto.Util.Padding import pad, unpad


def des_key(string: str) -> bytes:
    length = len(string)
    if length not in [14, 28]:
        raise ValueError("Key size is not valid")

    return bytes(int(string[i:i+2], 16) for i in range(0, length, 2))


def is_bitwise_odd(number: int) -> bool:
    ret = False
    while number != 0:
        if number & 1:
            ret = not ret
        number >>= 1

    return ret


def get_cipher(key: bytes) -> object:
    if 8 == len(key):
        return DES.new(key, DES.MODE_CBC) # nosec

    if 16 == len(key):
        return DES3.new(key, DES3.MODE_CBC)


parser = ArgumentParser(
    prog="des",
    description="Lavender utility for encoding/decoding files using DES cipher",
    epilog="Copyright (c) Mateusz Karcz, 2023-2024. Shared under the MIT License.")
parser.add_argument("command", choices=["encrypt", "decrypt"])
parser.add_argument("input", type=FileType("rb"))
parser.add_argument("output", type=FileType("wb"))
parser.add_argument("key", type=des_key)
args = parser.parse_args()

key_size = len(args.key) // 2
key_num = int.from_bytes(args.key, "big")
key_parts = [((key_num >> (i * 7)) & 0x7F) << 1 for i in reversed(range((key_size + 1) * 2))]
key_bytes = bytes(b if is_bitwise_odd(b) else b | 1 for b in key_parts)
print(*[f"{b:02x}" for b in key_bytes], sep="")

cipher = get_cipher(key_bytes)

if args.command == "encrypt":
    data = args.input.read()
    ct = cipher.encrypt(pad(data, DES.block_size))
    args.output.write(cipher.iv)
    args.output.write(ct)

if args.command == "decrypt":
    ivct = args.input.read()
    data = unpad(cipher.decrypt(ivct)[8:], DES.block_size)
    args.output.write(data)
