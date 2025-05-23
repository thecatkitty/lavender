Push-Location "C:\Program Files\Microsoft Platform SDK"
cmd.exe /c "SetEnv.Cmd /SRV64 /RETAIL & set" | ForEach-Object {
    if ($_ -match "=") {
        $Name, $Value = $_.Split("=", 2)
        Set-Item -Force -Path "Env:\$Name" -Value $Value
    } else {
        $Line
    }
}
Pop-Location
