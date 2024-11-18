#!/usr/bin/python3

from argparse import ArgumentParser, FileType

from lavender.cipher import des, xor

CIPHERS = {
    6: xor,
    7: des,
    14: des,
}


def raw_key(string: str) -> bytes:
    return int(string, 16).to_bytes(len(string) // 2, "big")


parser = ArgumentParser(
    prog="crypt",
    description="Lavender utility for encrypting/decrypting files",
    epilog="Copyright (c) Mateusz Karcz, 2021-2024. Shared under the MIT License.")
parser.add_argument("command", choices=["encrypt", "decrypt"])
parser.add_argument("input", type=FileType("rb"))
parser.add_argument("output", type=FileType("wb"))
parser.add_argument("key", type=raw_key, help="raw hexadecimal key")
args = parser.parse_args()

if args.command == "decrypt":
    CIPHERS[len(args.key)].decrypt(args.input, args.output, args.key)

if args.command == "encrypt":
    print(CIPHERS[len(args.key)].encrypt(args.input, args.output, args.key))
