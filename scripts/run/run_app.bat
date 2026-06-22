@echo off
setlocal
cd /d "%~dp0..\.."

if not exist "build-mingw\restaurant_mvp.exe" (
    call "%~dp0..\build\build.bat"
    if errorlevel 1 exit /b 1
)

.\build-mingw\restaurant_mvp.exe %*
