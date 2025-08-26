#!/usr/bin/env python3
import sys, csv


def read_summary(path):
    with open(path, newline="") as f:
        r = csv.DictReader(f)
        row = next(r)
        return {k: float(row[k]) for k in row}


def main():
    if len(sys.argv) != 2:
        print("usage: energy_compare.py <power_out/<timestamp>>", file=sys.stderr)
        sys.exit(2)
    d = sys.argv[1].rstrip("/")
    off = f"{d}/OFF/summary.csv"
    on  = f"{d}/ON/summary.csv"
    o = read_summary(off)
    n = read_summary(on)

    def pct(a,b): return (a/b*100.0) if b>0 else 0.0

    print("\n=== Power/Energy Comparison (Liquid FFT ON vs OFF) ===")
    print(f"Folder: {d}\n")
    print(f"{'Metric':<26}{'OFF':>14}{'ON':>14}{'Δ (ON-OFF)':>14}{'ON/OFF %':>12}")
    print("-"*80)
    for k, unit in [
        ("pps_avg","pps"),
        ("avg_power_mw","mW"),
        ("energy_per_packet_mJ","mJ/pkt"),
        ("energy_j","J"),
        ("duration_s","s"),
    ]:
        off_v = o[k]; on_v = n[k]
        print(f"{k+' ['+unit+']':<26}{off_v:>14.3f}{on_v:>14.3f}{(on_v-off_v):>14.3f}{pct(on_v,off_v):>12.2f}")
    print("")


if __name__ == "__main__":
    main()
