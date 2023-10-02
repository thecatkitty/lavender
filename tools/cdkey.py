#!/usr/bin/python3

import re
from argparse import ArgumentParser

BASE24_ALPHABET = "2346789BCDFGHJKMPQRTVWXY"
GROUP_PATTERN = "[" + BASE24_ALPHABET + "]{5}"
PRODUCT_KEY_PATTERN = GROUP_PATTERN + "(-" + GROUP_PATTERN + "){4}"

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
    if len(string) != 16:
        raise ValueError("User data has a wrong length")

    ret = bytes(int(string[i:i+2], 16) for i in range(0, 16, 2))
    if ret[0] > 127:
        raise ValueError("First byte of the user data too big")

    return ret

parser = ArgumentParser(
    prog="cdkey",
    description="Lavender utility for encoding/decoding 25-character CD keys",
    epilog="Copyright (c) Mateusz Karcz, 2023. Shared under the MIT License.")

subparsers = parser.add_subparsers(dest="subparser_name", required=True)

parser_decode = subparsers.add_parser("decode", help="decode CD key")
parser_decode.add_argument("cdkey", type=formatted_cdkey)

parser_encode = subparsers.add_parser("encode", help="encode raw key + user data")
parser_encode.add_argument("key", type=des56_key)
parser_encode.add_argument("data", type=user_data)

args = parser.parse_args()

if args.subparser_name == "decode":
    left_part, right_part = args.cdkey[:13], args.cdkey[13:]

    encoded_key = base24_decode(right_part).to_bytes(7, "big")
    data = base24_decode(left_part).to_bytes(8, "big")
    key = bytes(k ^ d for k, d in zip(encoded_key, data[1:]))

    print(*[f"{b:02x}" for b in key], sep="", end=" ")
    print(*[f"{b:02x}" for b in data], sep="")

if args.subparser_name == "encode":
    left_part = base24_encode(int.from_bytes(args.data, "big")).rjust(13, BASE24_ALPHABET[0])

    encoded_key = bytes(k ^ d for k, d in zip(args.key, args.data[1:]))
    right_part = base24_encode(int.from_bytes(encoded_key, "big")).rjust(12, BASE24_ALPHABET[0])

    cdkey = left_part + right_part
    print(f"{cdkey[:5]}-{cdkey[5:10]}-{cdkey[10:15]}-{cdkey[15:20]}-{cdkey[20:]}")
