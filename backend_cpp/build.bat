@echo off
echo ===============================================
echo Building Cash Futures THV C++ Backend
echo ===============================================

REM Create build directory
if not exist build mkdir build
cd build

echo Configuring with CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release

if %ERRORLEVEL% NEQ 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
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
echo Executable: build/Release/cash_futures_thv.exe
echo ===============================================
pause