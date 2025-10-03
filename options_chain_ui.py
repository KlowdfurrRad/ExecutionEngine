import dash
from dash import Dash, html, dcc, dash_table, Input, Output, State
import pandas as pd

# Sample dummy option chain data
data = {
    'Strike': [100, 105, 110, 115, 120],
    'Call Bid': [5.1, 3.4, 2.2, 1.1, 0.5],
    'Call Ask': [5.5, 3.8, 2.6, 1.4, 0.8],
    'Call Delta': [None]*5,
    'Call Gamma': [None]*5,
    'Put Bid': [0.6, 1.3, 2.5, 3.7, 5.0],
    'Put Ask': [0.9, 1.6, 2.9, 4.1, 5.4],
    'Put Delta': [None]*5,
    'Put Gamma': [None]*5
}

df = pd.DataFrame(data)

# Build Dash app
app = Dash(__name__)

app.layout = html.Div([
    html.H2("Options Chain Viewer"),

    dash_table.DataTable(
        id='options-table',
        columns=[
            {"name": "Strike", "id": "Strike"},
            {"name": "Call Bid", "id": "Call Bid"},
            {"name": "Call Ask", "id": "Call Ask"},
            {"name": "Call Delta", "id": "Call Delta"},
            {"name": "Call Gamma", "id": "Call Gamma"},
            {"name": "Put Bid", "id": "Put Bid"},
            {"name": "Put Ask", "id": "Put Ask"},
            {"name": "Put Delta", "id": "Put Delta"},
            {"name": "Put Gamma", "id": "Put Gamma"},
        ],
        data=df.to_dict('records'),
        style_table={'overflowX': 'auto'},
        style_cell={'textAlign': 'center', 'padding': '5px'},
        style_header={'backgroundColor': '#f4f4f4', 'fontWeight': 'bold'}
    ),

    html.Br(),

    html.Div([
        html.Label("Underlying Symbol:"),
        dcc.Input(id='underlying-input', type='text', value='AAPL', style={'marginRight': '20px'}),

        html.Label("Expiry (YYYY-MM-DD):"),
        dcc.Input(id='expiry-input', type='text', value='2025-10-15')
    ], style={'marginTop': '20px'})
])

if __name__ == '__main__':
    app.run(debug=True)