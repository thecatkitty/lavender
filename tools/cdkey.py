#!/usr/bin/python3

import re
from argparse import ArgumentParser

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


def formatted_cdkey(string: str) -> str:
    m = re.match(PRODUCT_KEY_PATTERN, string)
    if m is None:
        raise ValueError("CD key has a wrong format")

    return string.replace("-", "")


def des56_key(string: str) -> bytes:
    if len(string) != 14:
        raise ValueError("Key is not 56-bit")

    return bytes(int(string[i:i+2], 16) for i in range(0, 14, 2))


def user_data(string: str) -> bytes:
    if len(string) == 16:  # XOR 12 characters
        ret = bytes(int(string[i:i+2], 16) for i in range(0, 16, 2))
        if ret[0] > 127:
            raise ValueError("First byte of the user data too big")

        return ret

    if len(string) == 14:  # XOR 2 bits
        return des56_key(string)

    raise ValueError("User data has a wrong length")


def to_cdkey(left: int, right: int) -> str:
    head = base24_encode(left).rjust(13, BASE24_ALPHABET[0])
    tail = base24_encode(right).rjust(12, BASE24_ALPHABET[0])
    full = head + tail
    return f"{full[:5]}-{full[5:10]}-{full[10:15]}-{full[15:20]}-{full[20:]}"


parser = ArgumentParser(
    prog="cdkey",
    description="Lavender utility for encoding/decoding 25-character CD keys",
    epilog="Copyright (c) Mateusz Karcz, 2023-2024. Shared under the MIT License.")

subparsers = parser.add_subparsers(dest="subparser_name", required=True)

parser_decode = subparsers.add_parser("decode", help="decode CD key")
parser_decode.add_argument("cdkey", type=formatted_cdkey)
parser_decode.add_argument("--xor2b", action="store_true")

parser_encode = subparsers.add_parser(
    "encode", help="encode raw key + user data")
parser_encode.add_argument("key", type=des56_key)
parser_encode.add_argument("data", type=user_data)

args = parser.parse_args()

if args.subparser_name == "decode":
    left_part = base24_decode(args.cdkey[:13])
    right_part = base24_decode(args.cdkey[13:])

    if args.xor2b:
        x = PKEY25XOR2B_MASK[(left_part >> 57) & 3]
        k1 = ((left_part & ((1 << 56) - 1)) ^ x).to_bytes(7, "big")
        k2 = ((right_part | ((left_part >> 1) & (1 << 55))) ^ x).to_bytes(7, "big")
        print(*[f"{b:02x}" for b in k1], sep="", end=" ")
        print(*[f"{b:02x}" for b in k2], sep="")
        exit()

    encoded_key = right_part.to_bytes(7, "big")
    data = left_part.to_bytes(8, "big")
    key = bytes(k ^ d for k, d in zip(encoded_key, data[1:]))
    print(*[f"{b:02x}" for b in key], sep="", end=" ")
    print(*[f"{b:02x}" for b in data], sep="")

if args.subparser_name == "encode":
    if len(args.data) == 8:
        left_part = int.from_bytes(args.data, "big")
        encoded_key = bytes(k ^ d for k, d in zip(args.key, args.data[1:]))
        right_part = int.from_bytes(encoded_key, "big")
        print(to_cdkey(left_part, right_part))

    if len(args.data) == 7:
        k1 = int.from_bytes(args.key, "big")
        k2 = int.from_bytes(args.data, "big")
        for x in PKEY25XOR2B_MASK:
            k1x = k1 ^ x
            k2x = k2 ^ x
            left_part = ((x & 3) << 57) | ((k2x & (1 << 55)) << 1) | k1x
            right_part = k2x & ((1 << 55) - 1)
            print(to_cdkey(left_part, right_part))
