@echo off
setlocal

call clean.cmd

set vc="%VS140COMNTOOLS%..\..\VC\vcvarsall.bat"
if exist %vc% goto build
echo Microsoft Visual C++ 2015 is missing. Please go to http://www.visualstudio.com/ and install free Community edition.
exit /b 1
:build
echo Building...
call %vc% x86
msbuild SLAP.sln /v:m /t:Rebuild /p:Configuration=Release /p:Platform=x86
if exist Win32\release\SLAP.exe goto pack
echo Binary file is missing. Compile project first please.
exit /b 1

:pack
md "..\redist" 2>nul:
move /y "Win32\Release\slap_*.exe" "..\redist"

echo Done.
exit /b 0