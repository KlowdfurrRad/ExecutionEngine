@echo off
echo ===============================================
echo Building Cash Futures THV C++ Simple Backend
echo (No external dependencies - standard libs only)
echo ===============================================

REM Create build directory
if not exist build_simple mkdir build_simple
cd build_simple

echo Configuring with CMake...
cmake .. -f ../CMakeLists_simple.txt -DCMAKE_BUILD_TYPE=Release

if %ERRORLEVEL% NEQ 0 (
    echo CMake configuration failed!
    echo.
    echo Alternative: Direct compilation with g++/MSVC
    echo Trying direct compilation...
    cd ..
    
    REM Try direct compilation
    echo Compiling directly...
    g++ -std=c++17 -O2 -o cash_futures_thv_simple.exe main_simple.cpp -lws2_32
    
    if %ERRORLEVEL% EQU 0 (
        echo ===============================================
        echo Direct compilation successful!
        echo Executable: cash_futures_thv_simple.exe
        echo ===============================================
        pause
        exit /b 0
    ) else (
        echo Direct compilation also failed.
        echo Please ensure you have:
        echo 1. Visual Studio C++ tools OR MinGW-w64
        echo 2. CMake 3.10+
        pause
        exit /b 1
    )
)

echo Building project...
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo ===============================================
echo Build completed successfully!
echo Executable: build_simple/bin/cash_futures_thv_simple.exe
echo ===============================================
pause