#!/usr/bin/env python3
import csv, sys, os

def read_pps(csv_path):
    with open(csv_path, newline='') as f:
        r = csv.DictReader(f)
        row = next(r)
        return float(row['packets_per_sec'])

def main():
    if len(sys.argv) != 2:
        print("usage: bench_compare.py <bench_out/<timestamp>>", file=sys.stderr)
        sys.exit(2)
    d = sys.argv[1]
    off = os.path.join(d, "bench_OFF.csv")
    on  = os.path.join(d, "bench_ON.csv")
    if not (os.path.isfile(off) and os.path.isfile(on)):
        print(f"missing CSVs in {d} (expected bench_OFF.csv and bench_ON.csv)", file=sys.stderr)
        sys.exit(3)
    pps_off = read_pps(off)
    pps_on  = read_pps(on)
    delta   = pps_on - pps_off
    ratio   = (pps_on/pps_off)*100.0 if pps_off>0 else 0.0

    print("")
    print("=== LoRa Lite Benchmark Comparison ===")
    print(f"Folder: {d}")
    print("")
    print(f"{'Config':<18}{'packets_per_sec':>16}")
    print(f"{'-'*34}")
    print(f"{'FFT=OFF':<18}{pps_off:>16.3f}")
    print(f"{'FFT=ON':<18}{pps_on:>16.3f}")
    print(f"{'-'*34}")
    print(f"{'Δ (ON-OFF)':<18}{delta:>16.3f}")
    print(f"{'Ratio (ON/OFF)':<18}{ratio:>15.2f}%")

if __name__ == "__main__":
    main()
