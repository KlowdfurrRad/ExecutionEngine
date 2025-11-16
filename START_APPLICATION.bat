@echo off
echo ===============================================
echo Starting Cash Futures THV Trading Application
echo ===============================================
echo.

REM Check if Node.js is installed
node --version >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Node.js is not installed or not in PATH
    echo Please install Node.js from https://nodejs.org/
    pause
    exit /b 1
)

echo Node.js detected. Starting application...
echo.

REM Install backend dependencies if needed
echo Installing backend dependencies...
cd backend
if not exist node_modules (
    npm install
)
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to install backend dependencies
    pause
    exit /b 1
)

REM Start backend server in background
echo Starting backend server on port 8001...
start "Backend Server" cmd /k "npm run dev"

REM Wait a moment for backend to start
timeout /t 3 /nobreak >nul

REM Install frontend dependencies if needed
echo Installing frontend dependencies...
cd ..\frontend
if not exist node_modules (
    npm install
)
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to install frontend dependencies
    pause
    exit /b 1
)

REM Start frontend development server
echo Starting frontend application...
echo.
echo ===============================================
echo Application will open in your browser at:
echo http://localhost:3000
echo.
echo Backend API running at: http://localhost:8001
echo ===============================================
echo.
echo Press Ctrl+C in any window to stop the application
echo.

npm start

echo.
echo Application stopped.
pause