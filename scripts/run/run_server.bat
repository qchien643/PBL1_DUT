@echo off
setlocal
set "PORT=%~1"
if "%PORT%"=="" set "PORT=8080"
call "%~dp0run_app.bat" server "%PORT%"
