@echo off
setlocal
cd /d "%~dp0..\.."

echo Stopping Casual Dining MVP windows...

taskkill /F /IM restaurant_mvp.exe /T >nul 2>nul

taskkill /F /FI "WINDOWTITLE eq Restaurant Cashier*" /T >nul 2>nul
taskkill /F /FI "WINDOWTITLE eq Restaurant Customer T01*" /T >nul 2>nul
taskkill /F /FI "WINDOWTITLE eq Restaurant Kitchen*" /T >nul 2>nul
taskkill /F /FI "WINDOWTITLE eq Restaurant Bar*" /T >nul 2>nul
taskkill /F /FI "WINDOWTITLE eq Restaurant Manager*" /T >nul 2>nul
taskkill /F /FI "WINDOWTITLE eq Restaurant Server*" /T >nul 2>nul

if not exist "build-mingw\restaurant_mvp.exe" (
    call "%~dp0..\build\build.bat"
    if errorlevel 1 exit /b 1
)

echo Resetting data to demo seed...
call "%~dp0reset_data.bat"

echo.
echo Done. All app windows were closed and data was reset.
