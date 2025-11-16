@echo off
echo ===============================================
echo Running Cash Futures THV C++ Backend
echo ===============================================

REM Check if executable exists
if not exist "build\Release\cash_futures_thv.exe" (
    echo Executable not found! Please build the project first using build.bat
    echo.
    echo Run: build.bat
    pause
    exit /b 1
)

echo Starting C++ Backend Server...
echo Server will run on http://localhost:8002
echo Press Ctrl+C to stop the server
echo.

REM Run the executable
build\Release\cash_futures_thv.exe

echo.
echo Server stopped.
pause