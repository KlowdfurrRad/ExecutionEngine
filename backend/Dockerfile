# Multi-stage build for C++ backend
FROM ubuntu:22.04 AS builder

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libboost-all-dev \
    libpqxx-dev \
    libhiredis-dev \
    libcurl4-openssl-dev \
    libjsoncpp-dev \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Build the application
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# Production stage
FROM ubuntu:22.04 AS production

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libboost-system1.74.0 \
    libboost-thread1.74.0 \
    libboost-chrono1.74.0 \
    libpqxx-6.4 \
    libhiredis0.14 \
    libcurl4 \
    libjsoncpp25 \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

# Create app user
RUN useradd -r -s /bin/false trading

# Set working directory
WORKDIR /app

# Copy built application
COPY --from=builder /app/build/TradingPlatform /app/
COPY --from=builder /app/config/ /app/config/

# Change ownership
RUN chown -R trading:trading /app

# Switch to app user
USER trading

# Expose port
EXPOSE 8080

# Health check
HEALTHCHECK --interval=30s --timeout=3s --start-period=10s --retries=3 \
  CMD curl -f http://localhost:8080/health || exit 1

# Start the application
CMD ["./TradingPlatform"]