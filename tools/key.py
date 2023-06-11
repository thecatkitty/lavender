#!/usr/bin/python3

import re
from argparse import ArgumentParser

KEY_LOW_PART = 0
KEY_LOW_PART_MASK = 0b111111111111111111111111
KEY_HIGH_PART = 24
KEY_HIGH_PART_MASK = 0b111111111111111111111111

KEY_ROTATION_OFFSETS = [3, 7, 13, 19]

DISKID_HIGH_ROTATION = 30
DISKID_HIGH_ROTATION_MASK = 0b11
DISKID_HIGH_PART = 24
DISKID_HIGH_PART_MASK = 0b111111
DISKID_LOW_PART = 0
DISKID_LOW_PART_MASK = 0b111111111111111111111111

SECRET_LOW_ROTATION = 18
SECRET_LOW_ROTATION_MASK = 0b11
SECRET_HIGH_PART = 0
SECRET_HIGH_PART_MASK = 0b111111111111111111
SECRET_HIGH_PART_SHIFT = 6

def disk_id(string: str) -> int:
    m = re.match("([0-9a-f]{4})-([0-9a-f]{4})", string, re.I)
    if m is None:
        raise ValueError("Disk ID has a wrong format")

    return int(m.group(1) + m.group(2), 16)

def secret(string: str) -> int:
    if len(string) != 6:
        raise ValueError("Secret has a wrong length")

    return int(string, 10)

def xor48_key(string: str) -> int:
    if len(string) != 12:
        raise ValueError("Key is not 48-bit")
    return int(string, 16)

def rotl24(n: int, c: int) -> int:
    return (n << c | n >> (24 - c)) & 0xFFFFFF

def rotr24(n: int, c: int) -> int:
    return (n >> c | n << (24 - c)) & 0xFFFFFF

parser = ArgumentParser(
    prog="key",
    description="Lavender utility for encoding/decoding 48-bit keys",
    epilog="Copyright (c) Mateusz Karcz, 2021-2023. Shared under the MIT License.")

subparsers = parser.add_subparsers(dest="subparser_name", required=True)

parser_decode = subparsers.add_parser("decode", help="decode disk ID + secret")
parser_decode.add_argument("disk_id", type=disk_id)
parser_decode.add_argument("secret", type=secret)

parser_encode = subparsers.add_parser("encode", help="encode raw key")
parser_encode.add_argument("key", type=xor48_key)

args = parser.parse_args()

if args.subparser_name == "decode":
    low_part = (args.disk_id >> DISKID_LOW_PART) & DISKID_LOW_PART_MASK
    high_part = (((args.secret >> SECRET_HIGH_PART) & SECRET_HIGH_PART_MASK) << SECRET_HIGH_PART_SHIFT) | ((args.disk_id >> DISKID_HIGH_PART) & DISKID_HIGH_PART_MASK)

    low_rotation_index = (args.secret >> SECRET_LOW_ROTATION) & SECRET_LOW_ROTATION_MASK
    high_rotation_index = (args.disk_id >> DISKID_HIGH_ROTATION) & DISKID_HIGH_ROTATION_MASK

    low_part = rotr24(low_part, KEY_ROTATION_OFFSETS[low_rotation_index])
    high_part = rotr24(high_part, KEY_ROTATION_OFFSETS[high_rotation_index])

    key = ((high_part & KEY_HIGH_PART_MASK) << KEY_HIGH_PART) | ((low_part & KEY_LOW_PART_MASK) << KEY_LOW_PART)
    print(f"{key:012X}")

if args.subparser_name == "encode":
    low_part = (args.key >> KEY_LOW_PART) & KEY_LOW_PART_MASK
    high_part = (args.key >> KEY_HIGH_PART) & KEY_HIGH_PART_MASK

    for i in range(16):
        low_rotation_index = i % len(KEY_ROTATION_OFFSETS)
        high_rotation_index = i // len(KEY_ROTATION_OFFSETS)

        rot_low_part = rotl24(low_part, KEY_ROTATION_OFFSETS[low_rotation_index])
        rot_high_part = rotl24(high_part, KEY_ROTATION_OFFSETS[high_rotation_index])

        disk_id_part = ((high_rotation_index & DISKID_HIGH_ROTATION_MASK) << DISKID_HIGH_ROTATION) | ((rot_high_part & DISKID_HIGH_PART_MASK) << DISKID_HIGH_PART) | ((rot_low_part & DISKID_LOW_PART_MASK) << DISKID_LOW_PART)
        secret_part = ((low_rotation_index & SECRET_LOW_ROTATION_MASK) << SECRET_LOW_ROTATION) | (((rot_high_part >> SECRET_HIGH_PART_SHIFT) & SECRET_HIGH_PART_MASK) << SECRET_HIGH_PART)

        if secret_part >= 1000000:
            continue

        print(f"{disk_id_part >> 16:04X}-{disk_id_part & 0xFFFF:04X}\t{secret_part:06d}")
