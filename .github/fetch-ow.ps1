curl.exe -L "https://static.celones.pl/ci/lavender/resource_hacker.zip" -o ext/reshack.zip
curl.exe -L "https://static.celones.pl/ci/lavender/watcom-1.7-dec-alpha-crosscompiler.tar.xz" -o ext/watcom.txz
7z e ext/reshack.zip -oext/reshack
7z e ext/watcom.txz -oext
7z x ext/watcom.tar -oC:\
Remove-Item C:\WATCOM-axp\binnt\rc.exe
