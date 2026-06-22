@echo off
setlocal
set "STATION=%~1"
if "%STATION%"=="" set "STATION=kitchen"
call "%~dp0run_app.bat" kitchen "%STATION%"
