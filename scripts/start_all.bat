@echo off
setlocal
cd /d "%~dp0.."

if not exist "build-mingw\restaurant_mvp.exe" (
    call "%~dp0build.bat"
    if errorlevel 1 exit /b 1
)

set "APP=%CD%\build-mingw\restaurant_mvp.exe"

echo Starting all Casual Dining MVP windows...

start "Cashier CMD" cmd /k ""%APP%" cashier"
start "Customer T01 CMD" cmd /k ""%APP%" customer T01"
start "Kitchen CMD" cmd /k ""%APP%" kitchen kitchen"
start "Bar CMD" cmd /k ""%APP%" kitchen bar"
start "Manager CMD" cmd /k ""%APP%" manager"

echo.
echo Started:
echo - Cashier
echo - Customer T01
echo - Kitchen
echo - Bar
echo - Manager
