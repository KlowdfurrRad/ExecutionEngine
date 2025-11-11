# Cash Futures THV Trading Application - Complete Setup Guide

## Overview
This is a comprehensive trading application that replicates Excel-based Cash Futures Theoretical Value calculations with real-time Bloomberg data integration. The application includes:

- **Frontend**: React-based UI with Excel-like functionality
- **Backend Options**: Choose from Node.js, Python, or C++ backends
- **Real-time Data**: WebSocket streaming for live market updates
- **Financial Calculations**: Black-Scholes, SDV levels, theoretical value pricing

## Prerequisites

### General Requirements
- **Node.js 18+** (for frontend and Node.js backend)
- **Git** (for cloning dependencies)
- **Modern Web Browser** (Chrome, Firefox, Edge)

### Backend-Specific Requirements

#### For Node.js Backend (Recommended for Development)
- Node.js 18+ with npm

#### For Python Backend
- **Python 3.8-3.13** (avoid 3.14+ due to compatibility issues)
- pip package manager

#### For C++ Backend (High Performance)
- **Visual Studio 2019/2022** with C++ development tools
- **CMake 3.15+**
- **vcpkg** package manager

## Quick Start (Node.js Backend - Easiest)

### 1. Install Frontend Dependencies
```bash
cd frontend
npm install
```

### 2. Install Backend Dependencies
```bash
cd backend
npm install
```

### 3. Configure Environment
Copy the environment file:
```bash
cd backend
copy .env.example .env
```

Edit `.env` file with your Bloomberg API credentials (or use demo mode).

### 4. Start Backend Server
```bash
cd backend
npm run dev
```
Server runs on: http://localhost:8001

### 5. Start Frontend (New Terminal)
```bash
cd frontend
npm start
```
Frontend runs on: http://localhost:3000

### 6. Access Application
Open browser to: **http://localhost:3000**

## Detailed Setup Instructions

### Option 1: Node.js Backend (Recommended)

#### Step 1: Install Dependencies
```bash
# Frontend
cd frontend
npm install

# Backend
cd backend
npm install
```

#### Step 2: Configuration
```bash
cd backend
copy .env.example .env
```

Edit the `.env` file:
```
BLOOMBERG_API_KEY=your_api_key_here
BLOOMBERG_API_SECRET=your_secret_here
PORT=8001
CORS_ORIGIN=http://localhost:3000
```

#### Step 3: Start Services
Terminal 1 (Backend):
```bash
cd backend
npm run dev
```

Terminal 2 (Frontend):
```bash
cd frontend
npm start
```

### Option 2: Python Backend

#### Step 1: Setup Python Environment
```bash
# Create virtual environment (recommended)
python -m venv venv

# Activate virtual environment
# Windows:
venv\Scripts\activate
# macOS/Linux:
source venv/bin/activate

# Install dependencies
cd backend_python
pip install -r requirements_simple.txt
```

#### Step 2: Configure Environment
```bash
cd backend_python
copy .env.example .env
```

Edit the `.env` file with your Bloomberg credentials.

#### Step 3: Start Services
Terminal 1 (Python Backend):
```bash
cd backend_python
python main_simple.py
```
Server runs on: http://localhost:8000

Terminal 2 (Frontend):
```bash
cd frontend
npm install
npm start
```

Update frontend to use Python backend by editing `frontend/src/App.js`:
```javascript
const API_BASE_URL = 'http://localhost:8000';
```

### Option 3: C++ Backend (High Performance)

#### Step 1: Install vcpkg (if not installed)
```bash
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
```

#### Step 2: Install C++ Dependencies
```bash
C:\vcpkg\vcpkg install crow[ssl]:x64-windows
C:\vcpkg\vcpkg install nlohmann-json:x64-windows
```

#### Step 3: Build C++ Backend
```bash
cd backend_cpp
build.bat
```

#### Step 4: Configure and Run
Edit `backend_cpp/config.json` with your settings, then:

Terminal 1 (C++ Backend):
```bash
cd backend_cpp
run.bat
```
Server runs on: http://localhost:8002

Terminal 2 (Frontend):
```bash
cd frontend
npm install
npm start
```

Update frontend to use C++ backend by editing `frontend/src/App.js`:
```javascript
const API_BASE_URL = 'http://localhost:8002';
```

## Features Verification Checklist

### ✅ Core Features Implemented
- [x] Excel-like data grid with sorting, filtering
- [x] Real-time market data updates via WebSocket
- [x] Black-Scholes option pricing calculations
- [x] Standard Deviation Level (1σ, 2σ, 3σ) calculations
- [x] Theoretical value calculations
- [x] Color-coded value changes (green/red)
- [x] Column resizing and reordering
- [x] Basket management with weightings
- [x] Configuration panel for risk parameters
- [x] Export/Import functionality (CSV, JSON)
- [x] Customization panel (fonts, colors, themes)
- [x] Bloomberg API integration framework
- [x] Multi-backend architecture (Node.js, Python, C++)

### 📊 Financial Calculations
- [x] **Black-Scholes Formula**: Complete implementation with Greeks
- [x] **Standard Deviation Levels**: 1σ, 2σ, 3σ calculations
- [x] **Theoretical Value**: Futures pricing with interest rates
- [x] **Risk Metrics**: Delta, Gamma, Theta, Vega calculations
- [x] **PnL Calculations**: Real-time profit/loss tracking

### 🎨 UI/UX Features
- [x] **Excel-like Interface**: Matches original spreadsheet layout
- [x] **Color Scheme**: Green/red value indicators, professional styling
- [x] **Responsive Design**: Works on desktop and tablet devices
- [x] **Drag & Drop**: Column reordering and basket management
- [x] **Keyboard Navigation**: Excel-like keyboard shortcuts
- [x] **Context Menus**: Right-click options for rows and columns

### 🔄 Real-time Features
- [x] **WebSocket Streaming**: Live market data updates
- [x] **Auto-refresh**: Configurable update intervals
- [x] **Connection Status**: Visual indicators for data connectivity
- [x] **Error Handling**: Graceful degradation when data unavailable

## API Endpoints Reference

### Market Data
- `GET /api/market-data` - Get current market data
- `GET /api/instruments` - Get available instruments
- `POST /api/subscribe` - Subscribe to real-time updates

### Calculations
- `POST /api/calculate` - Calculate theoretical values
- `POST /api/black-scholes` - Black-Scholes pricing
- `POST /api/sdv-levels` - Standard deviation calculations

### Configuration
- `GET /api/config` - Get current configuration
- `POST /api/config` - Update configuration
- `POST /api/baskets` - Manage trading baskets

### WebSocket Events
- `market_update` - Real-time price updates
- `calculation_result` - Computed theoretical values
- `status_change` - Connection/system status updates

## Troubleshooting

### Common Issues

#### Frontend Won't Start
```bash
# Clear npm cache and reinstall
cd frontend
npm cache clean --force
rm -rf node_modules package-lock.json
npm install
```

#### Backend Connection Issues
1. Check if backend server is running on correct port
2. Verify CORS settings in backend configuration
3. Check firewall/antivirus blocking the ports

#### Python Backend Issues
```bash
# Use the simplified requirements
pip install -r requirements_simple.txt

# If still issues, install manually:
pip install fastapi uvicorn websockets python-dotenv
```

#### C++ Backend Build Issues
1. Ensure Visual Studio C++ tools are installed
2. Check vcpkg integration: `vcpkg integrate install`
3. Verify CMake is in PATH: `cmake --version`

### Performance Optimization

#### For High-Frequency Trading
- Use **C++ backend** for lowest latency (<1ms calculations)
- Enable **WebSocket compression** in production
- Configure **connection pooling** for Bloomberg API
- Use **Redis caching** for frequently accessed data

#### For Development
- Use **Node.js backend** for fastest development cycles
- Enable **hot reloading** in frontend
- Use **mock data mode** to avoid API rate limits

## Production Deployment

### Environment Variables
Create production `.env` files:
```bash
# Backend
BLOOMBERG_API_KEY=prod_api_key
BLOOMBERG_API_SECRET=prod_secret
PORT=8001
NODE_ENV=production
CORS_ORIGIN=https://your-domain.com

# Database (if using)
DATABASE_URL=postgresql://user:pass@host:port/db
REDIS_URL=redis://host:port
```

### Security Considerations
- Use HTTPS in production
- Implement API rate limiting
- Add authentication/authorization
- Validate all user inputs
- Use environment variables for secrets

### Monitoring
- Add logging for all API calls
- Monitor WebSocket connection health
- Track calculation performance metrics
- Set up alerts for system failures

## Next Steps

1. **Configure Bloomberg API**: Add your real API credentials
2. **Customize Calculations**: Modify formulas in backend calculation modules
3. **Add Instruments**: Extend the instrument list in configuration
4. **Implement Authentication**: Add user management if required
5. **Deploy to Production**: Use Docker or cloud platform deployment

## Support

For issues or questions:
1. Check the troubleshooting section above
2. Review backend-specific README files
3. Check console logs for error messages
4. Verify all prerequisites are installed correctly

## Architecture Overview

```
Frontend (React)
    ↓ HTTP/WebSocket
Backend (Node.js/Python/C++)
    ↓ API Calls
Bloomberg Data Provider
    ↓ Market Data
Real-time Calculations Engine
```

The application is designed with a modular architecture allowing easy swapping between backend implementations based on performance requirements and deployment constraints.