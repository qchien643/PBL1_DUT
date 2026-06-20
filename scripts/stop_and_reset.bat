@echo off
setlocal
cd /d "%~dp0.."

echo Stopping Casual Dining MVP windows...

taskkill /F /IM restaurant_mvp.exe /T >nul 2>nul

taskkill /F /FI "WINDOWTITLE eq Cashier CMD*" /T >nul 2>nul
taskkill /F /FI "WINDOWTITLE eq Customer T01 CMD*" /T >nul 2>nul
taskkill /F /FI "WINDOWTITLE eq Kitchen CMD*" /T >nul 2>nul
taskkill /F /FI "WINDOWTITLE eq Bar CMD*" /T >nul 2>nul
taskkill /F /FI "WINDOWTITLE eq Manager CMD*" /T >nul 2>nul

if not exist "build-mingw\restaurant_mvp.exe" (
    call "%~dp0build.bat"
    if errorlevel 1 exit /b 1
)

echo Resetting data to demo seed...
.\build-mingw\restaurant_mvp.exe reset

echo.
echo Done. All app windows were closed and data was reset.
