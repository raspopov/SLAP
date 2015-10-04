@echo off
call :clean .
call :clean Setup
call :clean SLAP
call :fileclean *.aps
call :fileclean *.ncb
call :fileclean *.sdf
exit /b 0

:clean
call :dirclean %1\Win32
call :dirclean %1\x64
call :dirclean %1\ipch
exit /b 0

:dirclean
if exist "%1" echo Removing %1... && rd /s /q "%1"
exit /b 0

:fileclean
for /r %%i in (%1) do echo Removing %%i... && del /q "%%i"
exit /b 0