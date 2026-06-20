@echo off
setlocal
cd /d "%~dp0.."

echo Building Casual Dining Ordering MVP...
cmake -S . -B build-mingw -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER="D:/App/MSYS3/ucrt64/bin/g++.exe"
if errorlevel 1 (
    echo.
    echo Build configure failed. Check CMake, MinGW, or compiler path.
    exit /b 1
)

cmake --build build-mingw
if errorlevel 1 (
    echo.
    echo Build failed.
    exit /b 1
)

echo.
echo Build completed: build-mingw\restaurant_mvp.exe
