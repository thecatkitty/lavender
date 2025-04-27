import os.path
import re
import subprocess

from argparse import ArgumentParser


def output_file_type(path: str):
    directory = os.path.dirname(path)
    if directory and not os.path.exists(directory):
        os.makedirs(directory, exist_ok=True)

    return open(path, "w", encoding="utf-8")


parser = ArgumentParser(
    prog="strconv",
    description="Lavender utility for converting Windows string resources to C",
    epilog="Copyright (c) Mateusz Karcz, 2025. Shared under the MIT License.")
parser.add_argument("input", help="input Resource Compiler script")
parser.add_argument("output", help="output C file", type=output_file_type)
parser.add_argument("cc", help="C compiler")
parser.add_argument("ccarg", nargs="*", help="C compiler arguments")
parser.add_argument("--suffix", default="", help="array name suffix")
args = parser.parse_args()

cpp = subprocess.run(
    [args.cc, "-x", "c", "-E", args.input, *args.ccarg], capture_output=True, encoding="utf-8")

print("#include <limits.h>", file=args.output)
print("#include <nls.h>", file=args.output)

for line in cpp.stdout.splitlines():
    if line == "STRINGTABLE":
        print(f"nls_locstr STRINGS{args.suffix}", end="", file=args.output)

    elif line.startswith("LANGUAGE"):
        print(f"[] = ", file=args.output)

    elif m := re.match(r"^\s*(0x[\dA-F]+),\s*(\"[^\"]*\")$", line):
        print("{" + m[1] + ", " + m[2] + "},", file=args.output)

    elif line == "}":
        print("{UINT_MAX}", file=args.output)
        print("};", file=args.output)

    else:
        print(line, file=args.output)
