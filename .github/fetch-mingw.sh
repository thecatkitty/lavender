#!/bin/sh

set -e

LLVM_MINGW_VERSION=20261006

LLVM_MINGW_CRT=msvcrt
if [ "$1" = "arm64" ]; then
    LLVM_MINGW_CRT=ucrt
fi

LLVM_MINGW_NAME="llvm-mingw-$LLVM_MINGW_VERSION-$LLVM_MINGW_CRT-ubuntu-22.04-x86_64"

curl -L https://github.com/mstorsjo/llvm-mingw/releases/download/$LLVM_MINGW_VERSION/$LLVM_MINGW_NAME.tar.xz -o ext/$LLVM_MINGW_NAME.tar.xz
tar xf ext/$LLVM_MINGW_NAME.tar.xz -C ext/
mv ext/$LLVM_MINGW_NAME ext/llvm-mingw-$LLVM_MINGW_CRT
echo "$PWD/ext/llvm-mingw-$LLVM_MINGW_CRT/bin" >> $GITHUB_PATH

if [ "$1" = "ia32" ]; then
    for crt in $LLVM_MINGW_CRT crtdll; do
        BULWA_NAME="bulwa-REL-$LLVM_MINGW_VERSION-$crt"
        curl -L https://github.com/thecatkitty/bulwa/releases/download/REL-$LLVM_MINGW_VERSION/$BULWA_NAME.zip -o ext/$BULWA_NAME.zip
        unzip ext/$BULWA_NAME.zip -d ext/bulwa-$crt
    done
 
    curl -L https://prdownloads.sourceforge.net/libunicows/libunicows-1.1.1-mingw32.zip -o ext/libunicows.zip
    unzip -j ext/libunicows.zip -d ext/libunicows
fi
