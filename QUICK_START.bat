@echo off
title Cash Futures THV Trading Application
color 0A
echo.
echo ╔══════════════════════════════════════════════════════════════════╗
echo ║                 Cash Futures THV Trading Application             ║
echo ║                          Quick Start                             ║
echo ╚══════════════════════════════════════════════════════════════════╝
echo.

REM Check Node.js
node --version >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ❌ Node.js not found. Please install from https://nodejs.org/
    echo.
    pause
    exit /b 1
)

echo ✅ Node.js detected
echo.
echo 🔧 Setting up backend...

REM Setup backend
cd backend
if not exist node_modules (
    echo Installing backend dependencies...
    npm install --silent
    if %ERRORLEVEL% NEQ 0 (
        echo ❌ Backend setup failed
        pause
        exit /b 1
    )
)

echo ✅ Backend ready
echo.
echo 🚀 Starting backend server...

REM Start backend in new window
start "THV Backend (Port 8001)" cmd /c "npm run dev"

REM Wait for backend startup
echo ⏳ Waiting for backend to start...
timeout /t 3 >nul

echo.
echo 🔧 Setting up frontend...

REM Setup frontend
cd ..\frontend
if not exist node_modules (
    echo Installing frontend dependencies...
    npm install --silent
    if %ERRORLEVEL% NEQ 0 (
        echo ❌ Frontend setup failed
        pause
        exit /b 1
    )
)

echo ✅ Frontend ready
echo.
echo 🌐 Starting frontend application...
echo.
echo ╔══════════════════════════════════════════════════════════════════╗
echo ║  Application will open automatically in your browser at:        ║
echo ║                                                                  ║
echo ║  📱 Frontend: http://localhost:3000                             ║
echo ║  🔗 Backend:  http://localhost:8001                             ║
echo ║                                                                  ║
echo ║  💡 Features Available:                                         ║
echo ║  • Real-time market data                                        ║
echo ║  • Excel-like interface                                         ║
echo ║  • Financial calculations (Black-Scholes, SDV)                  ║
echo ║  • Basket management                                            ║
echo ║  • Export/Import functionality                                  ║
echo ║                                                                  ║
echo ║  🛑 To stop: Press Ctrl+C in any window                        ║
echo ╚══════════════════════════════════════════════════════════════════╝
echo.

REM Start frontend (this will open browser automatically)
npm start

echo.
echo 🛑 Application stopped. Thank you for using THV Trading App!
pause