#!/usr/bin/python3

from argparse import ArgumentParser, FileType
from itertools import count

def xor48_key(string: str) -> int:
    if len(string) != 12:
        raise ValueError("Key is not 48-bit")
    return int(string, 16)

parser = ArgumentParser(
    prog="xor",
    description="Lavender utility for encoding/decoding files using XOR48 cipher",
    epilog="Copyright (c) Mateusz Karcz, 2021-2023. Shared under the MIT License.")
parser.add_argument("input", type=FileType("rb"))
parser.add_argument("output", type=FileType("wb"))
parser.add_argument("key", type=xor48_key)
args = parser.parse_args()

key = args.key.to_bytes(6, "big")

for i in count():
    data = args.input.read(1)
    if not data:
        break

    args.output.write(bytes([data[0] ^ key[i % 6]]))
