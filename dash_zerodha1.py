import dash
from dash import dcc, html, Input, Output, State, dash_table
import pandas as pd

# Initialize Dash app
app = dash.Dash(__name__)

# --- Color Scheme ---
colors = {
    'background': '#1a1a1a',
    'text': '#d4d4d4',
    'primary': '#007bff',
    'table_header': '#2a2a2a',
    'table_cell': '#3a3a3a',
    'input_bg': '#2c2c2c',
    'border': '#444444'
}

# --- Main App Layout ---
app.layout = html.Div(style={'backgroundColor': colors['background'], 'color': colors['text'], 'fontFamily': 'Arial, sans-serif', 'padding': '20px'}, children=[
    
    html.H1(
        "Portfolio Trading App",
        style={'textAlign': 'center', 'color': colors['primary'], 'marginBottom': '30px'}
    ),

    html.Div([
        html.Label("Total Capital (INR):", style={'fontSize': '18px'}),
        dcc.Input(
            id="capital", 
            type="number", 
            placeholder="Enter total capital", 
            value=100000,
            style={
                'width': '100%', 
                'padding': '10px', 
                'marginTop': '5px',
                'backgroundColor': colors['input_bg'],
                'color': colors['text'],
                'border': f"1px solid {colors['border']}",
                'borderRadius': '5px'
            }
        ),
    ], style={"marginBottom": "20px", 'maxWidth': '500px', 'margin': 'auto'}),

    html.Div([
        html.H3("Stock Allocation", style={'textAlign': 'center', 'color': colors['text']}),
        dash_table.DataTable(
            id="stock-table",
            columns=[
                {"name": "Stock", "id": "Stock", "editable": True},
                {"name": "Weight (%)", "id": "Weight", "editable": True, "type": "numeric"},
            ],
            data=[
                {"Stock": "RELIANCE", "Weight": 30},
                {"Stock": "TCS", "Weight": 25},
                {"Stock": "HDFCBANK", "Weight": 20},
                {"Stock": "INFY", "Weight": 25},
            ],
            editable=True,
            row_deletable=True,
            style_table={"overflowX": "auto"},
            style_header={
                'backgroundColor': colors['table_header'],
                'fontWeight': 'bold',
                'border': f'1px solid {colors["border"]}'
            },
            style_cell={
                'backgroundColor': colors['table_cell'],
                'color': colors['text'],
                'border': f'1px solid {colors["border"]}',
                'textAlign': 'left',
                'padding': '10px'
            },
            style_data_conditional=[
                {
                    'if': {'row_index': 'odd'},
                    'backgroundColor': '#2a2a2a'
                }
            ]
        ),
        html.Button(
            "Add Row", 
            id="add-row", 
            n_clicks=0,
            style={
                'marginTop': '10px', 
                'backgroundColor': colors['primary'], 
                'color': 'white', 
                'border': 'none', 
                'padding': '10px 20px', 
                'borderRadius': '5px',
                'cursor': 'pointer'
            }
        )
    ], style={'maxWidth': '800px', 'margin': '40px auto'}),

    html.Div([
        html.Button(
            "Calculate Allocation", 
            id="calc-btn", 
            n_clicks=0,
            style={
                'backgroundColor': '#28a745', 
                'color': 'white', 
                'border': 'none', 
                'padding': '12px 24px', 
                'borderRadius': '5px',
                'cursor': 'pointer',
                'fontSize': '16px',
                'marginRight': '10px'
            }
        ),
        html.Button(
            "Place Orders", 
            id="trade-btn", 
            n_clicks=0,
            style={
                'backgroundColor': '#dc3545', 
                'color': 'white', 
                'border': 'none', 
                'padding': '12px 24px', 
                'borderRadius': '5px',
                'cursor': 'pointer',
                'fontSize': '16px'
            }
        ),
    ], style={'textAlign': 'center', 'margin': '30px 0'}),
    
    html.Div(id="output-table", style={'maxWidth': '800px', 'margin': '20px auto'}),
    html.Div(id="trade-status", style={'textAlign': 'center', 'marginTop': '20px', 'fontSize': '16px'})
])

# --- Callbacks ---

# Add row callback
@app.callback(
    Output("stock-table", "data"),
    Input("add-row", "n_clicks"),
    State("stock-table", "data"),
    State("stock-table", "columns"),
    prevent_initial_call=True
)
def add_row(n_clicks, rows, columns):
    rows.append({c['id']: '' for c in columns})
    return rows

# Calculate allocation callback
@app.callback(
    Output("output-table", "children"),
    Input("calc-btn", "n_clicks"),
    State("capital", "value"),
    State("stock-table", "data"),
    prevent_initial_call=True
)
def calculate_allocation(n_clicks, capital, rows):
    df = pd.DataFrame(rows)
    df["Weight"] = pd.to_numeric(df["Weight"], errors="coerce")
    df = df.dropna()

    if df.empty or capital is None or capital <= 0:
        return html.Div("Please enter valid capital and stock data.", style={'color': '#ffc107', 'textAlign': 'center'})

    total_weight = df["Weight"].sum()
    if total_weight == 0:
        return html.Div("Total weight cannot be zero.", style={'color': '#ffc107', 'textAlign': 'center'})
        
    df["Capital_Allocation"] = (df["Weight"] / total_weight) * capital

    # Placeholder: fetch live stock price via an API here
    # For demonstration, we'll use a placeholder price.
    df["Price"] = 1000  # <-- Replace with API call

    df["Quantity"] = (df["Capital_Allocation"] / df["Price"]).astype(int)

    return dash_table.DataTable(
        columns=[{"name": i, "id": i} for i in df.columns],
        data=df.to_dict("records"),
        style_table={"overflowX": "auto"},
        style_header={
            'backgroundColor': colors['table_header'],
            'fontWeight': 'bold',
            'border': f'1px solid {colors["border"]}'
        },
        style_cell={
            'backgroundColor': colors['table_cell'],
            'color': colors['text'],
            'border': f'1px solid {colors["border"]}',
            'textAlign': 'left',
            'padding': '10px'
        },
        style_data_conditional=[
            {
                'if': {'row_index': 'odd'},
                'backgroundColor': '#2a2a2a'
            }
        ]
    )

# Trade execution callback
@app.callback(
    Output("trade-status", "children"),
    Input("trade-btn", "n_clicks"),
    State("output-table", "children"),
    prevent_initial_call=True
)
def place_orders(n_clicks, table_data):
    if table_data is None:
        return html.Div("Please calculate allocation first before placing orders.", style={'color': '#ffc107'})

    # Placeholder: Integrate with a trading API (e.g., Zerodha) here
    return html.Div("[PLACEHOLDER] Orders would be formatted and sent to the trading API here.", style={'color': '#28a745'})


if __name__ == "__main__":
    app.run(debug=True)