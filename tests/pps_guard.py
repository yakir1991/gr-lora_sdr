#!/usr/bin/env python3
import os, re, subprocess, sys

def run(cmd):
    p = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, check=False)
    return p.returncode, p.stdout

def main():
    # Allow custom minimum via env; default is conservative
    min_pps = float(os.getenv("PPS_MIN", "7000"))

    rc, out = run([os.path.join(os.getcwd(), "bench_lora_chain"), "bench_results.csv"])
    if rc != 0:
        print(out)
        print("bench_lora_chain failed to run", file=sys.stderr)
        return 2

    m = re.search(r"packets_per_sec=([0-9]+\.?[0-9]*)", out)
    if not m:
        print(out)
        print("Could not parse packets_per_sec", file=sys.stderr)
        return 3

    pps = float(m.group(1))
    print(f"metric,value\npps,{pps:.3f}\nmin_pps,{min_pps:.3f}")
    return 0 if pps >= min_pps else 1

if __name__ == "__main__":
    sys.exit(main())

