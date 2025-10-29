# Dash application: pairwise correlation of period returns (close-to-close)
# Save this file as dash_ticker_correlation_app.py and run: python dash_ticker_correlation_app.py

import datetime as dt
import io
import traceback

import pandas as pd
import yfinance as yf
import plotly.express as px
from dash import Dash, dcc, html, Input, Output, State

# --- Helper functions -----------------------------------------------------

def download_close_prices(tickers, start_date, end_date):
    """Download close prices for given tickers and date range using yfinance.

    Returns a DataFrame where columns are tickers and index is date.
    """
    # yfinance accepts a space- or comma-separated string
    df = yf.download(tickers, start=start_date, end=end_date, progress=False)
    # For single ticker, yf returns a dataframe with columns; 'Close' is a Series in that case
    # Need to fix the above later
    close = pd.DataFrame()
    for COLUMN in df.columns:
        if COLUMN[0] == 'Close':
            close[COLUMN[1]] = df[COLUMN]
    # Ensure columns are ticker symbols
    close.columns.name = None
    close = close.sort_index()
    return close


def compute_period_returns(close_df, period_days):
    """Compute period returns as close_t / close_{t-period} - 1.

    close_df: DataFrame with dates index and ticker columns.
    period_days: integer number of trading rows to shift.
    """
    if period_days < 1:
        raise ValueError("period_days must be >= 1")
    # Shift by period_days rows (assumes trading days; user requested 'period (in days)')
    returns = close_df / close_df.shift(period_days) - 1
    # Remove rows where all returns are NaN (initial rows)
    returns = returns.dropna(how='all')
    return returns


# --- Dash app layout -----------------------------------------------------

app = Dash(__name__)
app.title = "Ticker Pairwise Period-Return Correlation"

app.layout = html.Div([
    html.H2("Pairwise correlation of close-to-close period returns"),

    html.Div([
        html.Label("Tickers (comma separated, e.g. AAPL, MSFT, GOOGL):"),
        dcc.Input(id='tickers-input', type='text', value='AAPL,MSFT,GOOGL,AMZN', style={'width': '100%'}),
    ], style={'margin-bottom': '10px'}),

    html.Div([
        html.Label('Date range:'),
        dcc.DatePickerRange(
            id='date-range',
            start_date=(dt.date.today() - dt.timedelta(days=365)).isoformat(),
            end_date=dt.date.today().isoformat(),
            display_format='YYYY-MM-DD'
        ),
    ], style={'margin-bottom': '10px'}),

    html.Div([
        html.Label('Period (in days, integer):'),
        dcc.Input(id='period-input', type='number', value=5, min=1, step=1),
    ], style={'margin-bottom': '10px', 'width': '200px'}),

    html.Button('Compute correlations', id='compute-btn', n_clicks=0),
    html.Div(id='status', style={'margin-top': '10px', 'color': 'darkred'}),

    html.Hr(),

    dcc.Loading(
        id='loading-1',
        children=[
            dcc.Graph(id='corr-heatmap', style={'height': '600px'}),
            html.Div(id='extra-output')
        ],
        type='default'
    ),

    html.Div([
        html.H4('Period returns sample (first 8 rows)'),
        dcc.Markdown(id='returns-sample')
    ], style={'margin-top': '20px'})
], style={'max-width': '980px', 'margin': '0 auto', 'padding': '20px'})


# --- Callbacks -----------------------------------------------------------

@app.callback(
    Output('corr-heatmap', 'figure'),
    Output('status', 'children'),
    Output('returns-sample', 'children'),
    Input('compute-btn', 'n_clicks'),
    State('tickers-input', 'value'),
    State('date-range', 'start_date'),
    State('date-range', 'end_date'),
    State('period-input', 'value')
)
def update_correlation(n_clicks, tickers_value, start_date, end_date, period_value):
    """Download prices, compute period returns and produce a heatmap of pairwise correlations.

    Returns: (figure, status_message, markdown_sample)
    """
    # Default empty figure
    empty_fig = px.imshow([[0]], x=[''], y=[''], text_auto=True)

    try:
        if not tickers_value:
            return empty_fig, 'Please provide at least one ticker.', ''

        # Parse tickers
        tickers = [t.strip().upper() for t in tickers_value.replace(',', ' ').split() if t.strip()]
        if len(tickers) < 1:
            return empty_fig, 'Please provide at least one ticker (comma separated).', ''

        # Validate dates
        if start_date is None or end_date is None:
            return empty_fig, 'Please select a valid start and end date.', ''

        if period_value is None:
            period_value = 1
        period_value = int(period_value)
        if period_value < 1:
            return empty_fig, 'Period must be an integer >= 1.', ''

        # Download
        close_df = download_close_prices(tickers, start_date, end_date)
        if close_df.empty:
            return empty_fig, f'No price data found for tickers {tickers} in the selected range.', ''

        # If yf returns a Series for single ticker, make it DataFrame
        if isinstance(close_df, pd.Series):
            close_df = close_df.to_frame()

        # Keep only requested tickers columns
        # (yfinance can return extra columns if tickers overlap; try to select intersection)
        cols = [c for c in tickers if c in close_df.columns]
        if not cols:
            # sometimes yfinance returns lowercase or different names; attempt to use original columns
            cols = list(close_df.columns)

        close_df = close_df[cols]

        # Compute period returns
        returns = compute_period_returns(close_df, period_value)
        if returns.empty:
            return empty_fig, f'After computing {period_value}-day period returns there is no data (too short range).', ''

        # Compute pairwise correlation matrix
        corr = returns.corr()

        # Build heatmap figure
        fig = px.imshow(
            corr,
            text_auto='.2f',
            zmin=-1,
            zmax=1,
            aspect='auto',
            labels=dict(x='Ticker', y='Ticker', color=f'{period_value}-day return correlation'),
            title=f'Pairwise correlation of {period_value}-day close-to-close returns ({start_date} to {end_date})'
        )
        fig.update_layout(autosize=True, margin=dict(l=40, r=40, t=70, b=40))

        # Sample of returns for user visibility
        sample_md = '```' + returns.head(8).to_csv(index=True) + '\n```'

        return fig, '', sample_md

    except Exception as e:
        tb = traceback.format_exc()
        return empty_fig, f'Error: {str(e)}\nSee server logs for details.', ''


# --- Run server ----------------------------------------------------------

if __name__ == '__main__':
    print('Starting Dash app on http://127.0.0.1:8050')
    print('Requires: dash, pandas, yfinance, plotly')
    app.run(debug=True)
