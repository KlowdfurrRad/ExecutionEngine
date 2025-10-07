export interface MarketData {
  symbol: string;
  price: number;
  bid: number;
  ask: number;
  volume: number;
  openInterest: number;
  change: number;
  changePercent: number;
  lastUpdated: Date;
  exchange: 'NSE' | 'BSE';
}

export interface OptionContract {
  symbol: string;
  underlying: string;
  strike: number;
  expiry: Date;
  optionType: 'CE' | 'PE';
  marketData: MarketData;
  greeks: OptionGreeks;
  impliedVolatility: number;
}

export interface OptionGreeks {
  delta: number;
  gamma: number;
  theta: number;
  vega: number;
  rho: number;
}

export interface OptionsChain {
  underlying: string;
  expiry: Date;
  spotPrice: number;
  calls: OptionContract[];
  puts: OptionContract[];
}

export interface PriceComparison {
  symbol: string;
  instrumentType: 'CASH' | 'FUTURE' | 'OPTION';
  nsePrice: number;
  bsePrice: number;
  fairValue: number;
  percentageDiff: number;
  isRecommended: boolean;
  liquidityScore: number;
  volume30Day: number;
  currentVolume: number;
  openInterest: number;
  exchange: 'NSE' | 'BSE';
  expiryDate?: Date;
}

export interface TradingStrategy {
  id: string;
  name: string;
  legs: StrategyLeg[];
  totalPremium: number;
  maxProfit: number;
  maxLoss: number;
  breakevens: number[];
  marginRequired: number;
}

export interface StrategyLeg {
  id: string;
  contract: OptionContract;
  quantity: number;
  side: 'BUY' | 'SELL';
  price: number;
}

export interface OrderRequest {
  symbol: string;
  quantity: number;
  price?: number;
  orderType: 'MARKET' | 'LIMIT';
  side: 'BUY' | 'SELL';
  instrumentType: 'CASH' | 'FUTURE' | 'OPTION';
  exchange: 'NSE' | 'BSE';
  productType: 'CNC' | 'MIS' | 'NRML';
}

export interface Order {
  id: string;
  request: OrderRequest;
  status: 'PENDING' | 'EXECUTED' | 'REJECTED' | 'CANCELLED' | 'PARTIAL';
  executedQuantity: number;
  executedPrice?: number;
  timestamp: Date;
  rejectionReason?: string;
}

export interface Position {
  symbol: string;
  quantity: number;
  averagePrice: number;
  currentPrice: number;
  pnl: number;
  pnlPercent: number;
  instrumentType: 'CASH' | 'FUTURE' | 'OPTION';
  exchange: 'NSE' | 'BSE';
}

export interface Portfolio {
  id: string;
  name: string;
  totalValue: number;
  availableCapital: number;
  totalPnl: number;
  totalPnlPercent: number;
  positions: Position[];
  marginUsed: number;
  marginAvailable: number;
}

export interface BasketConfig {
  id: string;
  name: string;
  underlying: string;
  stocks: BasketStock[];
  totalWeightage: number;
  targetValue: number;
}

export interface BasketStock {
  symbol: string;
  weightage: number;
  direction: 'LONG' | 'SHORT';
  currentPrice: number;
  targetQuantity: number;
  allocatedValue: number;
}

export interface ConditionalOrder {
  id: string;
  condition: string; // e.g., "((NIFTY > 20000) || (BANKNIFTY < 50000)) && (TIME > 09:30)"
  orders: OrderRequest[];
  isActive: boolean;
  triggeredAt?: Date;
}

export interface RiskMetrics {
  portfolioValue: number;
  maxDrawdown: number;
  var95: number; // Value at Risk 95%
  positionSizeViolations: string[];
  marginUtilization: number;
  concentrationRisk: number;
}