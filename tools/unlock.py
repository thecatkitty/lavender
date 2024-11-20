#!/usr/bin/python3

from argparse import ArgumentParser
import os
import sys
import time

from lavender import remote


def hex_bytes(string: str) -> bytes:
    return int(string, 16).to_bytes(len(string) // 2, "big")


parser = ArgumentParser(
    prog="key",
    description="Lavender utility for manual remote unlocking of content access",
    epilog="Copyright (c) Mateusz Karcz, 2024. Shared under the MIT License.")

subparsers = parser.add_subparsers(dest="subparser_name", required=True)

parser_grant = subparsers.add_parser("grant", help="generate access key")
parser_grant.add_argument("cid", type=hex_bytes,
                          help="content identifier (CRC32 of the plaintext)")
parser_grant.add_argument("uid", type=hex_bytes, help="user identifer")

parser_request = subparsers.add_parser("request", help="request access")
parser_request.add_argument("access_code", help="access code")

parser_handle = subparsers.add_parser("handle", help="handle request")
parser_handle.add_argument("code", help="request code")

args = parser.parse_args()

if args.subparser_name == "grant":
    assert len(args.cid) == 4
    assert len(args.uid) == 10
    cid, uid = int.from_bytes(args.cid, "big"), int.from_bytes(args.uid, "big")
    print(remote.to_access_code(cid, uid))

if args.subparser_name == "request":
    cid, uid = remote.from_access_code(args.access_code)
    stamp = int(time.time()) & 0xFFFFFFFF
    request_bytes = remote.to_request_bytes(cid, uid, stamp)

    print("Share the following code with the slideshow author's representative:")
    print()
    print(" ", remote.to_decimal_code(request_bytes))
    print()
    print("The code is valid for 15 minutes.")
    print()
    print("Enter the confirmation code received from the representative below:")
    print()
    response = input("> ")
    response_bytes = remote.from_decimal_code(response)
    key = remote.from_response(request_bytes, response_bytes)
    print()
    print("The decryption key is:")
    print()
    print(f"  {key:028X}")
    print()

if args.subparser_name == "handle":
    assert "LAV_KEY" in os.environ

    request_bytes = remote.from_decimal_code(args.code)
    cid, uid, stamp = remote.from_request_bytes(request_bytes)
    now = int(time.time()) & 0xFFFFFFFF

    if stamp - now > 2 * 60:
        print("The request is from the future!", file=sys.stderr)
        exit(1)

    if now - stamp > 15 * 60:
        print("The request is stale!", file=sys.stderr)
        exit(1)

    key = int(os.environ["LAV_KEY"], 16)
    response_bytes = remote.to_response(request_bytes, key)
    print(remote.to_decimal_code(response_bytes))
