"""
Dash app to input stock symbols + weightages, estimate quantities from a total capital value
and send buy/sell orders to Zerodha (Kite Connect) API.

Features:
- Add/remove stocks with weight percentage
- Input total capital (INR) and compute quantities using current LTP
- Preview generated orders
- Send MARKET orders (BUY/SELL) to KiteConnect

Security / setup notes (read before running):
- Install dependencies: pip install dash dash-bootstrap-components pandas kiteconnect
- Save your Kite API credentials as environment variables:
    KITE_API_KEY, KITE_API_SECRET, KITE_ACCESS_TOKEN
  or edit the config section below to directly provide them (not recommended).
- Access tokens expire and must be created via the Kite login / request token flow. This example
  expects a valid ACCESS_TOKEN.
- This app places real orders. Test carefully (use a small amount or sandbox if available).

Run: python dash_zerodha_app.py
Open http://127.0.0.1:8050
"""

from dash import Dash, html, dcc, Input, Output, State, callback_context
import dash_bootstrap_components as dbc
import dash_table
import pandas as pd
import os
import math
import logging
from kiteconnect import KiteConnect

# -----------------------------
# Configuration / Credentials
# -----------------------------
# Recommended: export the following environment variables before running
#   export KITE_API_KEY=your_api_key
#   export KITE_API_SECRET=your_api_secret
#   export KITE_ACCESS_TOKEN=your_access_token

KITE_API_KEY = os.environ.get("KITE_API_KEY", "")
KITE_API_SECRET = os.environ.get("KITE_API_SECRET", "")
KITE_ACCESS_TOKEN = os.environ.get("KITE_ACCESS_TOKEN", "")

# If you prefer, inline credentials can be set here (not recommended for production):
# KITE_API_KEY = "your_key"
# KITE_API_SECRET = "your_secret"
# KITE_ACCESS_TOKEN = "your_token"

# Default exchange and product choices
DEFAULT_EXCHANGE = "NSE"
DEFAULT_PRODUCT = "CNC"  # "CNC" for delivery, "MIS"/"NRML" for intraday

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# -----------------------------
# Helper functions interacting with Kite
# -----------------------------

def create_kite_client():
    if not KITE_API_KEY:
        raise ValueError("KITE_API_KEY not set in environment. Set it before running.")
    kite = KiteConnect(api_key=KITE_API_KEY)
    if KITE_ACCESS_TOKEN:
        kite.set_access_token(KITE_ACCESS_TOKEN)
    else:
        raise ValueError("KITE_ACCESS_TOKEN not set. Obtain a valid access token via Kite Connect login flow.")
    return kite


def fetch_ltp_for_symbols(kite, symbols):
    """
    symbols: list of strings like 'RELIANCE' or 'INFY'
    Returns dict mapping 'EXCHANGE:SYMBOL' -> ltp, using DEFAULT_EXCHANGE.
    """
    # Kite expects instrument tokens or exchange-tradingsymbol format: 'NSE:RELIANCE'
    ltp_request = {f"{DEFAULT_EXCHANGE}:{s}": None for s in symbols}
    try:
        ltp_data = kite.ltp(list(ltp_request.keys()))
        result = {}
        for key, val in ltp_data.items():
            if val and "last_price" in val:
                result[key] = val["last_price"]
            else:
                result[key] = None
        return result
    except Exception as e:
        logger.exception("Error fetching LTPs: %s", e)
        # Return None values to indicate failure
        return {k: None for k in ltp_request.keys()}


def calculate_quantities(df, total_capital, kite):
    """
    Compute integer quantities for each symbol based on weight% and LTP.
    - df: DataFrame with columns ['symbol', 'weight_pct']
    - total_capital: float (INR)
    Returns df copy with ltp and quantity columns and estimated allocation.
    """
    symbols = df['symbol'].str.upper().tolist()
    ltp_map = fetch_ltp_for_symbols(kite, symbols)
    ltp_list = []
    qty_list = []
    alloc_list = []
    for sym, wt in zip(symbols, df['weight_pct']):
        key = f"{DEFAULT_EXCHANGE}:{sym}"
        ltp = ltp_map.get(key)
        ltp_list.append(ltp)
        if ltp is None or ltp == 0:
            qty = 0
            alloc = 0
        else:
            alloc = total_capital * (wt / 100.0)
            qty = math.floor(alloc / ltp)
        qty_list.append(int(qty))
        alloc_list.append(float(alloc))
    out = df.copy()
    out['exchange_symbol'] = [f"{DEFAULT_EXCHANGE}:{s.upper()}" for s in df['symbol']]
    out['ltp'] = ltp_list
    out['allocation_inr'] = alloc_list
    out['quantity'] = qty_list
    return out


def place_market_order(kite, tradingsymbol, transaction_type, quantity, product=DEFAULT_PRODUCT):
    """Place a simple market order via KiteConnect. Returns the API response (dict) or raises."""
    if quantity <= 0:
        raise ValueError("Quantity must be > 0")
    return kite.place_order(
        tradingsymbol=tradingsymbol.split(':')[-1],  # pass symbol only
        exchange=tradingsymbol.split(':')[0],
        transaction_type=transaction_type,
        quantity=quantity,
        order_type="MARKET",
        product=product,
    )

# -----------------------------
# Dash App Layout
# -----------------------------

app = Dash(__name__, external_stylesheets=[dbc.themes.BOOTSTRAP])
server = app.server

initial_df = pd.DataFrame(columns=['symbol', 'weight_pct'])

app.layout = dbc.Container([
    dbc.Row(dbc.Col(html.H2("Zerodha (Kite) Order Sender — Dash"), width=12)),
    dbc.Row([
        dbc.Col([
            dbc.Label("Total Capital (INR)"),
            dbc.Input(id='total-capital', type='number', value=10000, min=1, step=1),
        ], width=3),
        dbc.Col([
            dbc.Label("Order Action"),
            dcc.Dropdown(id='order-action', options=[{'label':'BUY','value':'BUY'},{'label':'SELL','value':'SELL'}], value='BUY')
        ], width=3),
        dbc.Col([
            dbc.Label("Product"),
            dcc.Dropdown(id='product', options=[{'label':'CNC','value':'CNC'},{'label':'MIS','value':'MIS'},{'label':'NRML','value':'NRML'}], value=DEFAULT_PRODUCT)
        ], width=3),
        dbc.Col([
            dbc.Button("Fetch LTPs & Calculate Quantities", id='calc-btn', color='primary', className='mt-4')
        ], width=3)
    ], className='mt-3'),

    html.Hr(),

    dbc.Row([
        dbc.Col([
            dbc.Label("Add stock symbol and weight (%)"),
            dbc.InputGroup([
                dbc.Input(id='symbol-input', placeholder='E.g. RELIANCE', type='text'),
                dbc.Input(id='weight-input', placeholder='Weight in % (e.g. 20)', type='number', min=0, max=100, step=0.01),
                dbc.Button("Add", id='add-btn', color='secondary')
            ], className='mb-2')
        ], width=8),
        dbc.Col(width=4)
    ]),

    dbc.Row(dbc.Col([
        dash_table.DataTable(
            id='positions-table',
            columns=[
                {'name':'Symbol','id':'symbol','editable':True},
                {'name':'Weight %','id':'weight_pct','editable':True,'type':'numeric'},
                {'name':'Exchange:Symbol','id':'exchange_symbol'},
                {'name':'LTP','id':'ltp','type':'numeric'},
                {'name':'Allocation (INR)','id':'allocation_inr','type':'numeric'},
                {'name':'Quantity','id':'quantity','type':'numeric'},
            ],
            data=initial_df.to_dict('records'),
            editable=True,
            row_deletable=True,
            style_table={'overflowX':'auto'},
        )
    ])),

    html.Div(id='calc-feedback', className='mt-2'),

    dbc.Row([
        dbc.Col(dbc.Button("Preview Orders", id='preview-btn', color='info'), width=3),
        dbc.Col(dbc.Button("Send Orders to Zerodha", id='send-btn', color='danger'), width=3),
    ], className='mt-3'),

    html.Hr(),
    html.H4("Order Preview / Responses"),
    dcc.Markdown(id='preview-output', children='No preview yet.'),
    dcc.Markdown(id='response-output', children='No responses yet.'),

    # store positions client-side
    dcc.Store(id='positions-store', data=initial_df.to_dict('records')),
], fluid=True)

# -----------------------------
# Callbacks
# -----------------------------

@app.callback(
    Output('positions-store', 'data'),
    Output('positions-table', 'data'),
    Input('add-btn', 'n_clicks'),
    State('symbol-input', 'value'),
    State('weight-input', 'value'),
    State('positions-store', 'data'),
    prevent_initial_call=True
)
def add_position(n_clicks, symbol, weight, positions_data):
    ctx = callback_context
    if not ctx.triggered:
        raise Exception('No trigger')
    # basic validation
    if not symbol or weight is None:
        return positions_data, positions_data
    symbol = symbol.strip().upper()
    try:
        weight = float(weight)
    except:
        weight = 0.0
    positions = positions_data or []
    positions.append({'symbol': symbol, 'weight_pct': weight})
    return positions, positions


@app.callback(
    Output('positions-store', 'data'),
    Input('positions-table', 'data_timestamp'),
    State('positions-table', 'data'),
    State('positions-store', 'data'),
)
def sync_table(timestamp, table_data, store_data):
    # When user edits table directly, keep store in sync
    if timestamp is None:
        raise dash.exceptions.PreventUpdate
    return table_data


@app.callback(
    Output('preview-output', 'children'),
    Output('positions-table', 'data'),
    Output('calc-feedback', 'children'),
    Input('calc-btn', 'n_clicks'),
    State('positions-store', 'data'),
    State('total-capital', 'value'),
    prevent_initial_call=True
)
def calculate_and_preview(n_clicks, positions_data, total_capital):
    if not positions_data:
        return 'No positions defined.', positions_data, ''
    df = pd.DataFrame(positions_data)
    try:
        kite = create_kite_client()
    except Exception as e:
        logger.exception('Kite client creation failed')
        return f'Error creating Kite client: {e}', positions_data, ''
    try:
        calc_df = calculate_quantities(df, float(total_capital), kite)
        preview_md = "### Order Preview\n"
        for idx, row in calc_df.iterrows():
            preview_md += f"- {row['exchange_symbol']}: Weight {row['weight_pct']}% — LTP {row['ltp']} — Quantity {row['quantity']} — Allocation ₹{row['allocation_inr']:.2f}\n"
        return preview_md, calc_df.to_dict('records'), 'Quantities updated using live LTPs.'
    except Exception as e:
        logger.exception('Calculation failed')
        return f'Calculation failed: {e}', positions_data, ''


@app.callback(
    Output('response-output', 'children'),
    Input('send-btn', 'n_clicks'),
    State('positions-store', 'data'),
    State('order-action', 'value'),
    State('product', 'value'),
    prevent_initial_call=True
)
def send_orders(n_clicks, positions_data, order_action, product):
    if not positions_data:
        return 'No positions to send.'
    try:
        pass
        # kite = create_kite_client()
    except Exception as e:
        logger.exception('Kite client creation failed')
        return f'Error creating Kite client: {e}'

    df = pd.DataFrame(positions_data)
    responses = []
    for idx, row in df.iterrows():
        trad_sym = row.get('exchange_symbol') or f"{DEFAULT_EXCHANGE}:{row['symbol'].upper()}"
        qty = int(row.get('quantity') or 0)
        if qty <= 0:
            responses.append(f"Skipping {trad_sym}: quantity {qty}")
            continue
        try:
            # resp = place_market_order(kite, trad_sym, order_action, qty, product=product)
            responses.append(f"Order placed for {trad_sym} qty {qty}: {resp}")
        except Exception as e:
            logger.exception('Order failed')
            responses.append(f"Order failed for {trad_sym} qty {qty}: {e}")
    # join responses into a markdown block
    out = "\n".join([f"- {r}" for r in responses])
    return out


if __name__ == '__main__':
    app.run(debug=True)
