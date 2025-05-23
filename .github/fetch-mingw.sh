#!/bin/sh
curl -kL https://github.com/mstorsjo/llvm-mingw/releases/download/20241001/llvm-mingw-20241001-msvcrt-ubuntu-20.04-x86_64.tar.xz -o ext/llvm-mingw.txz
tar xf ext/llvm-mingw.txz -C ext/
mv ext/llvm-mingw-20241001-msvcrt-ubuntu-20.04-x86_64 ext/llvm-mingw
echo "$PWD/ext/llvm-mingw/bin" >> $GITHUB_PATH

if [ "$1" = "ia32" ]; then
    curl -kL https://github.com/thecatkitty/bulwa/releases/download/REL-20250208/bulwa-REL-20250208-i486.zip -o ext/bulwa.zip
    curl -kL http://prdownloads.sourceforge.net/libunicows/libunicows-1.1.1-mingw32.zip -o ext/libunicows.zip
    unzip ext/bulwa.zip -d ext/bulwa
    unzip -j ext/libunicows.zip -d ext/libunicows
fi
