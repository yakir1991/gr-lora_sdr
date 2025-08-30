#!/usr/bin/env python3
import sys, csv

def load(csv_path):
    vals = {}
    backend = None
    with open(csv_path, newline='') as f:
        r = csv.reader(f)
        for row in r:
            if not row: continue
            if row[0] == 'fft_backend':
                backend = row[1]
                continue
            if row[0] in ('cycles','bytes_allocated','packets_per_sec'):
                vals[row[0]] = float(row[1])
    return backend or 'unknown', vals

def main():
    if len(sys.argv) != 3:
        print("usage: compare_chain_csv.py bench_chain_OFF.csv bench_chain_ON.csv")
        return 2
    b0, v0 = load(sys.argv[1])
    b1, v1 = load(sys.argv[2])
    keys = ['packets_per_sec','cycles','bytes_allocated']
    print(f"metric         {b0:>10}     {b1:>10}    ratio(ON/OFF)")
    print("------------  ----------  ----------   ------------")
    for k in keys:
        a = v0.get(k)
        b = v1.get(k)
        if a is None or b is None:
            print(f"{k:<12}  {'-':>10}  {'-':>10}   {'-':>12}")
        else:
            ratio = (b/a) if a else float('inf')
            print(f"{k:<12}  {a:10.3f}  {b:10.3f}   {ratio:12.3f}")

if __name__ == '__main__':
    sys.exit(main())

