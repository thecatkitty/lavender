import re
import subprocess

from argparse import ArgumentParser, FileType

parser = ArgumentParser(
    prog="strconv",
    description="Lavender utility for converting Windows string resources to C",
    epilog="Copyright (c) Mateusz Karcz, 2025. Shared under the MIT License.")
parser.add_argument("input", help="input Resource Compiler script")
parser.add_argument("output", help="output C file",
                    type=FileType("w", encoding="utf-8"))
parser.add_argument("cc", help="C compiler")
parser.add_argument("ccarg", nargs="*", help="C compiler arguments")
args = parser.parse_args()

cpp = subprocess.run(
    [args.cc, "-x", "c", "-E", args.input, *args.ccarg], capture_output=True, encoding="utf-8")

print("#include <limits.h>", file=args.output)
print("#include <nls.h>", file=args.output)

for line in cpp.stdout.splitlines():
    if line == "STRINGTABLE":
        print("nls_locstr STRINGS", end="", file=args.output)

    elif line.startswith("LANGUAGE"):
        print(f"[] = ", file=args.output)

    elif m := re.match(r"^\s*(0x[\dA-F]+),\s*(\"[^\"]*\")$", line):
        print("{" + m[1] + ", " + m[2] + "},", file=args.output)

    elif line == "}":
        print("{UINT_MAX}", file=args.output)
        print("};", file=args.output)

    else:
        print(line, file=args.output)
