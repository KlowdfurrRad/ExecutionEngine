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
    html.H2("Options Chain Viewer", style={'textAlign': 'center', 'color': '#1f3c88'}),

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
        style_table={'overflowX': 'auto', 'border': '1px solid #ccc', 'margin': '20px'},
        style_cell={'textAlign': 'center', 'padding': '10px', 'fontFamily': 'Arial', 'fontSize': 14},
        style_header={'backgroundColor': '#1f3c88', 'color': 'white', 'fontWeight': 'bold'},
        style_data_conditional=[
            {
                'if': {'column_id': 'Call Bid'},
                'backgroundColor': '#e3f2fd'
            },
            {
                'if': {'column_id': 'Call Ask'},
                'backgroundColor': '#bbdefb'
            },
            {
                'if': {'column_id': 'Put Bid'},
                'backgroundColor': '#ffebee'
            },
            {
                'if': {'column_id': 'Put Ask'},
                'backgroundColor': '#ffcdd2'
            },
            {
                'if': {'column_id': 'Strike'},
                'backgroundColor': '#f5f5f5',
                'fontWeight': 'bold'
            }
        ]
    ),

    html.Br(),

    html.Div([
        html.Label("Underlying Symbol:", style={'marginRight': '10px', 'color': '#1f3c88'}),
        dcc.Input(id='underlying-input', type='text', value='AAPL', style={'marginRight': '30px'}),

        html.Label("Expiry (YYYY-MM-DD):", style={'marginRight': '10px', 'color': '#1f3c88'}),
        dcc.Input(id='expiry-input', type='text', value='2025-10-15')
    ], style={'marginTop': '20px', 'textAlign': 'center'})
], style={'backgroundColor': '#fafafa', 'padding': '20px'})

if __name__ == '__main__':
    app.run(debug=True)