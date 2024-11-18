#!/usr/bin/python3

from argparse import ArgumentParser

from lavender.key import LE32B6D


METHODS = {
    "le": LE32B6D
}


def raw_key(string: str) -> bytes:
    return int(string, 16).to_bytes(len(string) // 2, "big")


parser = ArgumentParser(
    prog="key",
    description="Lavender utility for encoding/decoding encryption keys",
    epilog="Copyright (c) Mateusz Karcz, 2021-2024. Shared under the MIT License.")
parser.add_argument("-m", choices=METHODS.keys(), required=True,
                    help="key transformation method")

subparsers = parser.add_subparsers(dest="subparser_name", required=True)

parser_decode = subparsers.add_parser("decode", help="decode formatted key")
parser_decode.add_argument("part", type=str, nargs="+",
                           help="formatted key part(s)")

parser_encode = subparsers.add_parser("encode", help="encode raw key")
parser_encode.add_argument("key", type=raw_key, help="raw hexadecimal key")
parser_encode.add_argument("parameter", type=str,
                           nargs="?", help="encoding parameter")

args = parser.parse_args()

if args.subparser_name == "decode":
    decoded = METHODS[args.m].decode(args.part)
    print("".join(f"{b:02X}" for b in decoded))

if args.subparser_name == "encode":
    le = METHODS[args.m](args.key)
    for fkey in le.encode(args.parameter):
        print("\t".join(fkey))
