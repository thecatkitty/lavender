curl.exe -kL https://download.microsoft.com/download/7/5/e/75ec7f04-4c8c-4f38-b582-966e76602643/5.2.3790.1830.15.PlatformSDK_Svr2003SP1_rtm.img -o ext/winsdk52.iso
7z x ext/winsdk52.iso -oext/winsdk52

"Installing Windows Server 2003 SP1 Platform SDK..."
Start-Process -Wait msiexec.exe -ArgumentList @("/i", (Get-Item .\ext\winsdk52\Setup\PSDK-amd64.msi).FullName, "ADDLOCAL=ALL", "/norestart", "/quiet")
