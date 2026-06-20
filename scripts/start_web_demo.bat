@echo off
setlocal
cd /d "%~dp0.."

if not exist "build-mingw\restaurant_mvp.exe" (
    call "%~dp0build.bat"
    if errorlevel 1 exit /b 1
)

start "C++ Restaurant Server" cmd /k ""%CD%\build-mingw\restaurant_mvp.exe" server 8080"

timeout /t 1 >nul

start "" "http://localhost:8080/"
start "" "http://localhost:8080/cashier.html"
start "" "http://localhost:8080/customer.html?table=T01"
start "" "http://localhost:8080/kitchen.html?station=kitchen"
start "" "http://localhost:8080/kitchen.html?station=bar"
start "" "http://localhost:8080/manager.html"

echo Web demo started at http://localhost:8080
