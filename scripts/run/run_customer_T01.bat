@echo off
setlocal
set "TABLE_CODE=%~1"
if "%TABLE_CODE%"=="" set "TABLE_CODE=T01"
call "%~dp0run_app.bat" customer "%TABLE_CODE%"
