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
            if row[0].startswith('fft_us_per_exec_sf'):
                sf = int(row[0].split('sf')[1])
                vals[sf] = float(row[1])
    return backend or 'unknown', vals

def main():
    if len(sys.argv) != 3:
        print("usage: compare_fft_csv.py bench_fft_OFF.csv bench_fft_ON.csv")
        return 2
    b0, v0 = load(sys.argv[1])
    b1, v1 = load(sys.argv[2])
    sfs = sorted(set(v0) | set(v1))
    print(f"SF  {b0:>10} (us)  {b1:>10} (us)   ratio(ON/OFF)")
    print("--  ----------  ----------   -----------")
    for sf in sfs:
        a = v0.get(sf)
        b = v1.get(sf)
        if a is None or b is None:
            print(f"{sf:<2}  {'-':>10}  {'-':>10}   {'-':>11}")
        else:
            ratio = b/a if a else float('inf')
            print(f"{sf:<2}  {a:10.3f}  {b:10.3f}   {ratio:11.3f}")

if __name__ == '__main__':
    sys.exit(main())

