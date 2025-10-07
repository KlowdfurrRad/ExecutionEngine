-- Trading Platform Database Schema
-- PostgreSQL initialization script

-- Create database extensions
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- Portfolios table
CREATE TABLE IF NOT EXISTS portfolios (
    id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    capital DECIMAL(15,2) NOT NULL DEFAULT 0,
    available_capital DECIMAL(15,2) NOT NULL DEFAULT 0,
    total_pnl DECIMAL(15,2) NOT NULL DEFAULT 0,
    margin_used DECIMAL(15,2) NOT NULL DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Baskets configuration
CREATE TABLE IF NOT EXISTS baskets (
    id SERIAL PRIMARY KEY,
    portfolio_id INTEGER REFERENCES portfolios(id) ON DELETE CASCADE,
    name VARCHAR(100) NOT NULL,
    underlying VARCHAR(50) NOT NULL,
    total_weightage DECIMAL(5,2) DEFAULT 100.00,
    target_value DECIMAL(15,2) DEFAULT 0,
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Basket stocks
CREATE TABLE IF NOT EXISTS basket_stocks (
    id SERIAL PRIMARY KEY,
    basket_id INTEGER REFERENCES baskets(id) ON DELETE CASCADE,
    symbol VARCHAR(50) NOT NULL,
    weightage DECIMAL(5,2) NOT NULL,
    direction VARCHAR(10) NOT NULL CHECK (direction IN ('LONG', 'SHORT')),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Orders table
CREATE TABLE IF NOT EXISTS orders (
    id SERIAL PRIMARY KEY,
    order_id VARCHAR(50) UNIQUE,
    portfolio_id INTEGER REFERENCES portfolios(id),
    symbol VARCHAR(50) NOT NULL,
    exchange VARCHAR(10) NOT NULL CHECK (exchange IN ('NSE', 'BSE')),
    instrument_type VARCHAR(20) NOT NULL CHECK (instrument_type IN ('CASH', 'FUTURE', 'OPTION')),
    product_type VARCHAR(10) NOT NULL CHECK (product_type IN ('CNC', 'MIS', 'NRML')),
    quantity INTEGER NOT NULL,
    price DECIMAL(10,2),
    order_type VARCHAR(10) NOT NULL CHECK (order_type IN ('MARKET', 'LIMIT')),
    side VARCHAR(10) NOT NULL CHECK (side IN ('BUY', 'SELL')),
    status VARCHAR(20) NOT NULL DEFAULT 'PENDING' CHECK (status IN ('PENDING', 'EXECUTED', 'REJECTED', 'CANCELLED', 'PARTIAL')),
    executed_quantity INTEGER DEFAULT 0,
    executed_price DECIMAL(10,2),
    rejection_reason TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    executed_at TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Market data table
CREATE TABLE IF NOT EXISTS market_data (
    id SERIAL PRIMARY KEY,
    symbol VARCHAR(50) NOT NULL,
    exchange VARCHAR(10) NOT NULL CHECK (exchange IN ('NSE', 'BSE')),
    instrument_type VARCHAR(20) NOT NULL,
    price DECIMAL(10,2) NOT NULL,
    bid DECIMAL(10,2),
    ask DECIMAL(10,2),
    volume BIGINT DEFAULT 0,
    open_interest BIGINT DEFAULT 0,
    change_value DECIMAL(10,2) DEFAULT 0,
    change_percent DECIMAL(5,2) DEFAULT 0,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Positions table
CREATE TABLE IF NOT EXISTS positions (
    id SERIAL PRIMARY KEY,
    portfolio_id INTEGER REFERENCES portfolios(id),
    symbol VARCHAR(50) NOT NULL,
    exchange VARCHAR(10) NOT NULL,
    instrument_type VARCHAR(20) NOT NULL,
    product_type VARCHAR(10) NOT NULL,
    quantity INTEGER NOT NULL DEFAULT 0,
    average_price DECIMAL(10,2) NOT NULL DEFAULT 0,
    current_price DECIMAL(10,2) NOT NULL DEFAULT 0,
    pnl DECIMAL(15,2) NOT NULL DEFAULT 0,
    pnl_percent DECIMAL(5,2) NOT NULL DEFAULT 0,
    margin_used DECIMAL(15,2) DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(portfolio_id, symbol, exchange, instrument_type, product_type)
);

-- Option contracts table
CREATE TABLE IF NOT EXISTS option_contracts (
    id SERIAL PRIMARY KEY,
    symbol VARCHAR(50) NOT NULL,
    underlying VARCHAR(50) NOT NULL,
    strike DECIMAL(10,2) NOT NULL,
    expiry DATE NOT NULL,
    option_type VARCHAR(2) NOT NULL CHECK (option_type IN ('CE', 'PE')),
    lot_size INTEGER NOT NULL DEFAULT 1,
    tick_size DECIMAL(4,2) DEFAULT 0.05,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(underlying, strike, expiry, option_type)
);

-- Price comparison history
CREATE TABLE IF NOT EXISTS price_comparisons (
    id SERIAL PRIMARY KEY,
    symbol VARCHAR(50) NOT NULL,
    instrument_type VARCHAR(20) NOT NULL,
    nse_price DECIMAL(10,2),
    bse_price DECIMAL(10,2),
    fair_value DECIMAL(10,2),
    percentage_diff DECIMAL(5,2),
    is_recommended BOOLEAN DEFAULT false,
    liquidity_score DECIMAL(3,2),
    volume_30day BIGINT DEFAULT 0,
    current_volume BIGINT DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Historical volume data (for 30-day averages)
CREATE TABLE IF NOT EXISTS historical_volumes (
    id SERIAL PRIMARY KEY,
    symbol VARCHAR(50) NOT NULL,
    exchange VARCHAR(10) NOT NULL,
    volume BIGINT NOT NULL,
    date DATE NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(symbol, exchange, date)
);

-- User sessions (for authentication)
CREATE TABLE IF NOT EXISTS user_sessions (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(100) NOT NULL,
    session_token VARCHAR(255) NOT NULL UNIQUE,
    expires_at TIMESTAMP NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- System configuration
CREATE TABLE IF NOT EXISTS system_config (
    id SERIAL PRIMARY KEY,
    config_key VARCHAR(100) NOT NULL UNIQUE,
    config_value TEXT NOT NULL,
    description TEXT,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Create indexes for performance
CREATE INDEX IF NOT EXISTS idx_orders_symbol ON orders(symbol);
CREATE INDEX IF NOT EXISTS idx_orders_status ON orders(status);
CREATE INDEX IF NOT EXISTS idx_orders_created_at ON orders(created_at);
CREATE INDEX IF NOT EXISTS idx_market_data_symbol ON market_data(symbol, exchange);
CREATE INDEX IF NOT EXISTS idx_market_data_timestamp ON market_data(timestamp);
CREATE INDEX IF NOT EXISTS idx_positions_portfolio ON positions(portfolio_id);
CREATE INDEX IF NOT EXISTS idx_historical_volumes_symbol ON historical_volumes(symbol, date);
CREATE INDEX IF NOT EXISTS idx_option_contracts_underlying ON option_contracts(underlying, expiry);

-- Insert default portfolio
INSERT INTO portfolios (name, capital, available_capital) 
VALUES ('Stenoa Capital Fund', 100000000.00, 100000000.00)
ON CONFLICT DO NOTHING;

-- Insert default system configuration
INSERT INTO system_config (config_key, config_value, description) VALUES
('risk_free_rate', '0.06', 'Risk-free interest rate for pricing calculations'),
('volume_threshold', '0.05', 'Maximum volume threshold (5% of 30-day average)'),
('market_open_time', '09:15', 'Market opening time'),
('market_close_time', '15:30', 'Market closing time'),
('max_position_size', '0.10', 'Maximum position size (10% of portfolio)')
ON CONFLICT (config_key) DO NOTHING;

-- Create functions for automatic timestamp updates
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$ language 'plpgsql';

-- Create triggers for updated_at columns
CREATE TRIGGER update_portfolios_updated_at BEFORE UPDATE ON portfolios 
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_baskets_updated_at BEFORE UPDATE ON baskets 
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_orders_updated_at BEFORE UPDATE ON orders 
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_positions_updated_at BEFORE UPDATE ON positions 
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- Grant permissions
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO trading;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO trading;