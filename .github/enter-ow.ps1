$WatcomPath = "C:\WATCOM-axp"

$env:PATH = (
    "$WatcomPath\BINNT",
    "$WatcomPath\BINW",
    "C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64",
    "$PWD\ext\reshack",
    $env:PATH) -join ";"
$env:INCLUDE = (
    "$WatcomPath\H",
    "$WatcomPath\H\NT",
    "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared") -join ";"
$env:WATCOM = $WatcomPath
$env:EDPATH = "$WatcomPath\EDDAT"
$env:LIB = "$WatcomPath\libaxp\nt"
