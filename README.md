# Stenoa Capital Executive Trading Platform

A sophisticated options trading execution platform built for proprietary fund management, featuring real-time market data integration, pricing comparison engine, and advanced risk management.

## 🚀 Features

### Phase 2 Implementation (Current)
- **Options Trading UI** - Complete options chain display with real-time Greeks
- **Pricing Comparison Engine** - NSE vs BSE price analysis with fair value calculations
- **Multi-leg Strategy Builder** - Create and execute complex options strategies
- **Real-time Market Data** - Live price feeds via Zerodha API integration
- **Risk Management** - Position size limits and margin calculations

### Core Capabilities
- **Smart Contract Selection** - Optimal futures vs cash selection based on liquidity
- **Volume Compliance** - 5% volume limit enforcement
- **Margin Optimization** - Real-time margin requirement calculations
- **Order Management** - Slice execution and exchange rule compliance
- **Portfolio Tracking** - Real-time P&L and position monitoring

## 🏗️ Architecture

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   Vue.js        │    │   C++ Backend    │    │   PostgreSQL    │
│   Frontend      │◄──►│   Trading Engine │◄──►│   Database      │
│   (Port 3000)   │    │   (Port 8080)    │    │   (Port 5432)   │
└─────────────────┘    └──────────────────┘    └─────────────────┘
         │                        │                        │
         │                        ▼                        │
         │              ┌──────────────────┐               │
         └─────────────►│   Redis Cache    │◄──────────────┘
                        │   (Port 6379)    │
                        └──────────────────┘
                                 │
                                 ▼
                        ┌──────────────────┐
                        │   Zerodha API    │
                        │   Integration    │
                        └──────────────────┘
```

## 📦 Project Structure

```
stenoa-trading-platform/
├── frontend/                 # Vue.js Frontend
│   ├── src/
│   │   ├── components/      # Trading UI components
│   │   │   ├── OptionsChain.vue
│   │   │   ├── PricingComparison.vue
│   │   │   └── TradingDashboard.vue
│   │   ├── types/          # TypeScript definitions
│   │   └── main.ts         # Application entry point
│   ├── package.json
│   └── Dockerfile
├── backend/                 # C++ Backend
│   ├── include/            # Header files
│   │   ├── pricing_engine.h
│   │   └── zerodha_connector.h
│   ├── src/               # Source files
│   │   └── pricing_engine.cpp
│   ├── CMakeLists.txt
│   └── Dockerfile
├── docker-compose.yml      # Container orchestration
├── technical_specification.md
├── implementation_roadmap.md
└── README.md
```

## 🚀 Quick Start

### Prerequisites
- Docker and Docker Compose
- Node.js 18+ (for local development)
- CMake and C++17 compiler (for local backend development)

### 1. Clone and Setup
```bash
git clone <repository-url>
cd stenoa-trading-platform
```

### 2. Environment Configuration
Create a `.env` file in the root directory:
```bash
# Zerodha API Credentials
ZERODHA_API_KEY=your_api_key_here
ZERODHA_API_SECRET=your_api_secret_here

# Security
JWT_SECRET=your_jwt_secret_here

# Database (optional - defaults provided)
POSTGRES_PASSWORD=trading123
REDIS_PASSWORD=redis123
```

### 3. Run with Docker Compose
```bash
# Start the complete platform
docker-compose up -d

# Start with monitoring (optional)
docker-compose --profile monitoring up -d

# For production with load balancer
docker-compose --profile production up -d
```

### 4. Access the Platform
- **Trading Platform**: http://localhost:3000
- **Backend API**: http://localhost:8080
- **Grafana Dashboard**: http://localhost:3001 (if monitoring enabled)
- **Prometheus**: http://localhost:9090 (if monitoring enabled)

## 🔧 Development Setup

### Frontend Development
```bash
cd frontend
npm install
npm run dev
# Access at http://localhost:3000
```

### Backend Development
```bash
cd backend
mkdir build && cd build
cmake ..
make -j$(nproc)
./TradingPlatform
```

## 📊 Trading Features

### Options Chain Display
- Real-time options prices for NIFTY, BANKNIFTY, FINNIFTY
- Interactive strike selection with Greeks display
- Implied volatility and open interest tracking
- Color-coded calls and puts with ATM highlighting

### Pricing Comparison Engine
- NSE vs BSE spot price comparison
- Current vs next month futures selection
- Fair value calculations using Black-Scholes model
- Liquidity scoring and volume compliance checking

### Strategy Builder
- Multi-leg options strategy construction
- Real-time P&L calculations
- Margin requirement estimation
- One-click strategy execution

### Risk Management
- Position size limits (5% of 30-day volume)
- Real-time portfolio monitoring
- Margin utilization tracking
- Conditional order execution

## 🔌 API Integration

### Zerodha Kite API
The platform integrates with Zerodha's Kite API for:
- Real-time market data
- Order placement and management
- Portfolio and position tracking
- Historical data access

### Configuration
Update your Zerodha API credentials in the environment variables:
```bash
export ZERODHA_API_KEY="your_api_key"
export ZERODHA_API_SECRET="your_secret"
```

## 🏦 Database Schema

### Core Tables
- `portfolios` - Fund portfolio management
- `baskets` - Trading basket configurations
- `orders` - Order lifecycle tracking
- `market_data` - Price and volume data
- `positions` - Current holdings

### Performance Optimizations
- Indexed queries for real-time data access
- Redis caching for frequently accessed data
- Connection pooling for high-throughput operations

## 📈 Monitoring & Logging

### Prometheus Metrics
- Order execution latency
- API response times
- Database query performance
- Memory and CPU usage

### Grafana Dashboards
- Real-time trading metrics
- Portfolio performance tracking
- System health monitoring
- Alert configurations

## 🔒 Security Features

### Authentication & Authorization
- JWT-based authentication
- API key management
- Session-based access control
- Audit logging

### Data Protection
- TLS encryption for all communications
- Database encryption at rest
- Secure API credential storage
- Rate limiting and DDoS protection

## 🧪 Testing

### Unit Tests
```bash
# Frontend tests
cd frontend && npm test

# Backend tests
cd backend/build && make tests && ./tests
```

### Integration Tests
```bash
# Run complete test suite
docker-compose -f docker-compose.test.yml up --abort-on-container-exit
```

## 📋 Phase 2 Deliverables Status

- ✅ **Options Trading UI** - Complete with real-time chain display
- ✅ **Pricing Comparison Logic** - NSE/BSE analysis with fair value
- ✅ **Strike Selection Interface** - Interactive Greeks and IV display
- ✅ **Order Entry System** - Quick order and strategy execution
- ✅ **Real-time Data Integration** - WebSocket-based price feeds
- ✅ **Risk Metrics Display** - Margin and position monitoring
- ✅ **Containerized Deployment** - Docker-based microservices

## 🚦 Production Deployment

### Cloud Infrastructure
```bash
# AWS/Azure/GCP deployment
docker-compose --profile production up -d

# Scale services
docker-compose up --scale backend=3 --scale frontend=2
```

### Performance Tuning
- Backend: Optimize C++ for low-latency execution
- Frontend: Code splitting and lazy loading
- Database: Query optimization and indexing
- Cache: Redis configuration for market data

## 📝 Configuration

### Backend Configuration
Edit `backend/config/trading.conf`:
```ini
[api]
port=8080
max_connections=1000

[database]
host=postgres
port=5432
name=trading_db

[zerodha]
rate_limit=10
timeout=5000
```

### Frontend Configuration
Edit `frontend/vite.config.ts` for API endpoints and proxy settings.

## 🐛 Troubleshooting

### Common Issues
1. **API Connection Failed**: Check Zerodha API credentials and network connectivity
2. **Database Connection Error**: Verify PostgreSQL service status
3. **Frontend Build Failed**: Ensure Node.js version compatibility
4. **Backend Compilation Error**: Check C++ compiler and dependencies

### Logs Access
```bash
# View service logs
docker-compose logs -f backend
docker-compose logs -f frontend

# Database logs
docker-compose logs -f postgres
```

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/new-feature`)
3. Commit changes (`git commit -am 'Add new feature'`)
4. Push to branch (`git push origin feature/new-feature`)
5. Create Pull Request

## 📄 License

This project is proprietary software developed for Stenoa Capital. All rights reserved.

## 📞 Support

For technical support and questions:
- Internal Wiki: [Company Documentation]
- Email: tech@stenoaco.com
- Slack: #trading-platform

---

**Phase 2 Complete** ✅ - Options Trading UI and Pricing Engine Ready for Integration