#!/usr/bin/env bash

set -e

if [ "$#" -ne 2 ]; then
	echo "usage: $0 <winver> <binary>" >&2
	exit 1
fi

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
APILIST_FILE="$SCRIPT_DIR/winapi/${1,,}.txt"
BINARY_FILE="$2"

if [ ! -f "$APILIST_FILE" ]; then
	echo "$0: error: API list file '$APILIST_FILE' does not exist" >&2
	exit 1
fi

if [ ! -f "$BINARY_FILE" ]; then
	echo "$0: warning: File '$BINARY_FILE' does not exist" >&2
	exit 0 # not an error, just skip the check
fi

llvm-objdump -p "$BINARY_FILE" \
	| sed -ne '/The Import Tables/,$p' \
	| awk '
		/^[[:space:]]*DLL Name: / { dll = $3; next }
		/^[[:space:]]+[0-9]+[[:space:]]+[^[:space:]]+/ {
			print tolower(dll) "!" $2
		}
	' \
	| sort \
	| comm -23 - <(sort "$APILIST_FILE") \
	| awk ' \
		{ print $0; bad = 1 }
		END { exit bad }
	'
