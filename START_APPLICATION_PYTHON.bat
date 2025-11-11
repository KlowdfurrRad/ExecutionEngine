@echo off
echo ===============================================
echo Starting Cash Futures THV Trading Application
echo (Using Python Backend - No C++ Dependencies)
echo ===============================================
echo.

REM Check if Python is installed
python --version >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Python is not installed or not in PATH
    echo Please install Python from https://python.org/
    pause
    exit /b 1
)

REM Check if Node.js is installed
node --version >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Node.js is not installed or not in PATH
    echo Please install Node.js from https://nodejs.org/
    pause
    exit /b 1
)

echo Python and Node.js detected. Starting application...
echo.

REM Install Python backend dependencies if needed
echo Installing Python backend dependencies...
cd backend_python
if not exist venv (
    python -m venv venv
)
call venv\Scripts\activate
pip install -r requirements_simple.txt >nul 2>&1

REM Start Python backend server in background
echo Starting Python backend server on port 8000...
start "Python Backend Server" cmd /k "call venv\Scripts\activate && python main_simple.py"

REM Wait a moment for backend to start
timeout /t 4 /nobreak >nul

REM Install frontend dependencies if needed
echo Installing frontend dependencies...
cd ..\frontend
if not exist node_modules (
    npm install >nul 2>&1
)

REM Update frontend to use Python backend
echo Configuring frontend for Python backend...
powershell -Command "(Get-Content src\App.js) -replace 'localhost:8001', 'localhost:8000' | Set-Content src\App.js.tmp; Move-Item src\App.js.tmp src\App.js"

REM Start frontend development server
echo Starting frontend application...
echo.
echo ===============================================
echo Application will open in your browser at:
echo http://localhost:3000
echo.
echo Backend API running at: http://localhost:8000
echo ===============================================
echo.
echo Press Ctrl+C in any window to stop the application
echo.

npm start

echo.
echo Application stopped.
pause