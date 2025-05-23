#!/bin/sh
curl -kL https://github.com/thecatkitty/nicetia16/releases/download/REL-20250305/nicetia16-REL-20250305-$1.zip -o ext/nicetia16-$1.zip
curl -kL https://github.com/thecatkitty/andrea/releases/download/0.1.2/andrea-0.1.2.zip -o ext/andrea.zip
unzip ext/nicetia16-$1.zip -d ext/nicetia16-$1
unzip ext/andrea.zip -d ext/andrea
