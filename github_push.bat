@echo off
setlocal
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\github_publish.ps1" %*
set "publish_exit=%errorlevel%"
if not "%publish_exit%"=="0" echo GitHub update stopped. Exit code: %publish_exit%
if not "%publish_exit%"=="0" if "%~1"=="" pause
endlocal & exit /b %publish_exit%
