import dash
from dash import dcc, html, Input, Output, State, dash_table
import pandas as pd

# Initialize Dash app
app = dash.Dash(__name__)

# Layout
app.layout = html.Div([
    html.H1("Portfolio Trading App"),

    html.Div([
        html.Label("Total Capital (INR):"),
        dcc.Input(id="capital", type="number", placeholder="Enter total capital", value=100000),
    ], style={"margin": "10px 0"}),

    html.Div([
        html.H3("Stock Allocation"),
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
        ),
        html.Button("Add Row", id="add-row", n_clicks=0)
    ]),

    html.Br(),
    html.Button("Calculate Allocation", id="calc-btn", n_clicks=0),
    html.Div(id="output-table"),

    html.Br(),
    html.Button("Place Orders", id="trade-btn", n_clicks=0),
    html.Div(id="trade-status")
])

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

    total_weight = df["Weight"].sum()
    df["Capital_Allocation"] = (df["Weight"] / total_weight) * capital

    # Placeholder: fetch live stock price via Zerodha API here
    df["Price"] = 1000  # <-- Replace with Zerodha API call

    df["Quantity"] = (df["Capital_Allocation"] / df["Price"]).astype(int)

    return dash_table.DataTable(
        columns=[{"name": i, "id": i} for i in df.columns],
        data=df.to_dict("records"),
        style_table={"overflowX": "auto"}
    )

# Trade execution callback
@app.callback(
    Output("trade-status", "children"),
    Input("trade-btn", "n_clicks"),
    State("output-table", "children"),
    prevent_initial_call=True
)
def place_orders(n_clicks, table_data):
    # Placeholder: place trade orders via Zerodha API here
    return html.Div("[PLACEHOLDER] Orders would be sent to Zerodha API here.")


if __name__ == "__main__":
    app.run(debug=True)
