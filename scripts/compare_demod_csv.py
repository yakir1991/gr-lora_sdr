#!/usr/bin/env python3
import sys, csv

def load(csv_path):
    backend = None
    vals = {}
    with open(csv_path, newline='') as f:
        r = csv.reader(f)
        for row in r:
            if not row: continue
            if row[0] == 'fft_backend':
                backend = row[1]
                continue
            if row[0].startswith('demod_us_per_sym_sf'):
                key = row[0].split(',')[0]
                parts = key.split('_')
                sf = int(parts[4][2:])  # sfXX
                vals[sf] = float(row[1])
    return backend or 'unknown', vals

def main():
    if len(sys.argv) != 3:
        print("usage: compare_demod_csv.py bench_demod_OFF.csv bench_demod_ON.csv")
        return 2
    b0, v0 = load(sys.argv[1])
    b1, v1 = load(sys.argv[2])
    sfs = sorted(set(v0) | set(v1))
    print(f"SF  {b0:>10} (us/sym)  {b1:>10} (us/sym)   ratio(ON/OFF)")
    print("--  ---------------  ---------------   -----------")
    for sf in sfs:
        a = v0.get(sf)
        b = v1.get(sf)
        if a is None or b is None:
            print(f"{sf:<2}  {'-':>15}  {'-':>15}   {'-':>11}")
        else:
            ratio = b/a if a else float('inf')
            print(f"{sf:<2}  {a:15.3f}  {b:15.3f}   {ratio:11.3f}")

if __name__ == '__main__':
    sys.exit(main())

