
# comps_analysis.py
# pip install yfinance pandas matplotlib openpyxl
import yfinance as yf
import pandas as pd
import matplotlib.pyplot as plt

# FTSE 250 mid-cap peers (example)
PEERS = ["OCDO.L", "SGE.L", "AUTO.L", "MRO.L"]  # Ocado, Sage, Auto Trader, Melrose
TARGET = "TARGET.CO"  # placeholder

def get_metrics(ticker):
    stock = yf.Ticker(ticker)
    info = stock.info
    return {
        'Name': info.get('longName', ticker),
        'EV/EBITDA': info.get('enterpriseToEbitda', 0),
        'P/E': info.get('trailingPE', 0),
        'EV/Rev': info.get('enterpriseToRevenue', 0),
        'Market Cap (£m)': info.get('marketCap', 0) / 1e6
    }

def run_comps():
    data = []
    for t in PEERS:
        try:
            data.append(get_metrics(t))
        except:
            continue
    
    df = pd.DataFrame(data)
    df = df.round(2)
    
    # Stats
    median_ev_ebitda = df['EV/EBITDA'].median()
    
    print("=== FTSE COMPARABLE COMPANY ANALYSIS ===")
    print(df.to_string(index=False))
    print(f"\nMedian EV/EBITDA: {median_ev_ebitda:.1f}x")
    print("========================================\n")
    
    # Export
    df.to_excel("comps_output.xlsx", index=False)
    print("Exported: comps_output.xlsx")

    # Chart
    plt.figure(figsize=(8,4))
    plt.bar(df['Name'], df['EV/EBITDA'], color='#1f77b4')
    plt.axhline(median_ev_ebitda, color='red', linestyle='--', label=f'Median: {median_ev_ebitda:.1f}x')
    plt.title("EV/EBITDA Comps – FTSE Mid-Cap Peers")
    plt.ylabel("EV/EBITDA (x)")
    plt.xticks(rotation=45)
    plt.legend()
    plt.tight_layout()
    plt.savefig("comps_chart.png", dpi=300)
    print("Chart saved: comps_chart.png")

if __name__ == "__main__":
    run_comps()

