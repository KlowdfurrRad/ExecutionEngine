# Cash Futures THV C++ Backend

High-performance C++ backend implementation using the Crow framework for the Cash Futures Theoretical Value trading application.

## Prerequisites

- **Visual Studio 2019/2022** with C++ development tools
- **CMake 3.15+**
- **vcpkg** (for package management)

## Setup Instructions

### 1. Install vcpkg (if not already installed)
```bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
```

### 2. Install Dependencies
```bash
vcpkg install crow[ssl]:x64-windows
vcpkg install nlohmann-json:x64-windows
```

### 3. Build the Project
Run the build script:
```bash
build.bat
```

Or manually:
```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

### 4. Run the Server
```bash
run.bat
```

Or manually:
```bash
build\Release\cash_futures_thv.exe
```

## API Endpoints

- **GET** `/api/market-data` - Get current market data
- **POST** `/api/calculate` - Calculate theoretical values
- **WebSocket** `/ws` - Real-time data streaming

## Configuration

Edit `config.json` to customize:
- Server port (default: 8002)
- Bloomberg API settings
- Risk parameters
- Calculation settings

## Features

- High-performance financial calculations
- Real-time WebSocket streaming
- Black-Scholes option pricing
- Standard Deviation Level calculations
- Multi-threaded request handling
- CORS support for frontend integration

## Performance

This C++ backend is optimized for:
- Low latency calculations (<1ms)
- High throughput (1000+ req/sec)
- Memory efficient operations
- Real-time data processing