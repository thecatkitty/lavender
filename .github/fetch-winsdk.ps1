curl.exe -kL https://download.microsoft.com/download/7/5/e/75ec7f04-4c8c-4f38-b582-966e76602643/5.2.3790.1830.15.PlatformSDK_Svr2003SP1_rtm.img -o ext/winsdk52.iso
curl.exe -kL http://download.microsoft.com/download/F/1/0/F10113F5-B750-4969-A255-274341AC6BCE/GRMSDKX_EN_DVD.iso -o ext/winsdk71.iso
7z x ext/winsdk52.iso -oext/winsdk52
7z x ext/winsdk71.iso -oext/winsdk71

"Installing Windows Server 2003 SP1 Platform SDK..."
Start-Process -Wait msiexec.exe -ArgumentList @("/i", (Get-Item .\ext\winsdk52\Setup\PSDK-amd64.msi).FullName, "ADDLOCAL=ALL", "/norestart", "/quiet")

"Extracting Windows SDK 7.1 Resource Compiler..."
7z e ext/winsdk71/Setup/WinSDKWin32Tools_amd64/cab1.cab -oext/winsdk71 WinSDK_RC_Exe_24874546_57CF_4C5F_80BC_4348431DB664_amd64 WinSDK_RcDll_Dll_24874546_57CF_4C5F_80BC_4348431DB664_amd64
Move-Item .\ext\winsdk71\WinSDK_RC_Exe_24874546_57CF_4C5F_80BC_4348431DB664_amd64 .\ext\winsdk71\RC.Exe
Move-Item .\ext\winsdk71\WinSDK_RcDll_Dll_24874546_57CF_4C5F_80BC_4348431DB664_amd64 .\ext\winsdk71\RcDll.Dll
