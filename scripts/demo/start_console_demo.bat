@echo off
setlocal
cd /d "%~dp0..\.."

if not exist "build-mingw\restaurant_mvp.exe" (
    call "%~dp0..\build\build.bat"
    if errorlevel 1 exit /b 1
)

set "APP=%CD%\build-mingw\restaurant_mvp.exe"

echo Starting Casual Dining MVP console demo windows...

start "Restaurant Cashier" cmd /k ""%APP%" cashier"
start "Restaurant Customer T01" cmd /k ""%APP%" customer T01"
start "Restaurant Kitchen" cmd /k ""%APP%" kitchen kitchen"
start "Restaurant Bar" cmd /k ""%APP%" kitchen bar"
start "Restaurant Manager" cmd /k ""%APP%" manager"

echo.
echo Started console windows:
echo - Restaurant Cashier
echo - Restaurant Customer T01
echo - Restaurant Kitchen
echo - Restaurant Bar
echo - Restaurant Manager
