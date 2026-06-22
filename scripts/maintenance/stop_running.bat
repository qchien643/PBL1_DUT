@echo off
setlocal
cd /d "%~dp0..\.."

echo Stopping running Casual Dining MVP processes...

taskkill /F /IM restaurant_mvp.exe /T >nul 2>nul

taskkill /F /FI "WINDOWTITLE eq Restaurant Cashier*" /T >nul 2>nul
taskkill /F /FI "WINDOWTITLE eq Restaurant Customer T01*" /T >nul 2>nul
taskkill /F /FI "WINDOWTITLE eq Restaurant Kitchen*" /T >nul 2>nul
taskkill /F /FI "WINDOWTITLE eq Restaurant Bar*" /T >nul 2>nul
taskkill /F /FI "WINDOWTITLE eq Restaurant Manager*" /T >nul 2>nul
taskkill /F /FI "WINDOWTITLE eq Restaurant Server*" /T >nul 2>nul

echo Done. Running app processes were stopped.
