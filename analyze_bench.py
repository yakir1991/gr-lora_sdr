#!/usr/bin/env python3
import argparse
import csv
import statistics
import sys


def main():
    parser = argparse.ArgumentParser(description="Analyze benchmark CSV results")
    parser.add_argument("csv", nargs="?", default="bench_results.csv",
                        help="Path to bench_results.csv")
    parser.add_argument("--threshold", type=float, default=0.1,
                        help="Allowed fractional deviation before flagging")
    args = parser.parse_args()

    with open(args.csv, newline="") as f:
        reader = csv.DictReader(f)
        cycles = []
        bytes_alloc = []
        pps = []
        for row in reader:
            cycles.append(int(row["cycles"]))
            bytes_alloc.append(int(row["bytes_allocated"]))
            pps.append(float(row["packets_per_sec"]))

    avg_cycles = statistics.mean(cycles)
    avg_bytes = statistics.mean(bytes_alloc)
    avg_pps = statistics.mean(pps)
    print(f"Average cycles: {avg_cycles:.0f}")
    print(f"Average bytes allocated: {avg_bytes:.0f}")
    print(f"Average packets/sec: {avg_pps:.3f}")

    def max_dev(values, avg):
        return max(abs(v - avg) / avg for v in values) if values else 0.0

    flagged = False
    if cycles:
        dev = max_dev(cycles, avg_cycles)
        if dev > args.threshold:
            print(f"Cycle deviation {dev:.2%} exceeds threshold")
            flagged = True
    if bytes_alloc:
        dev = max_dev(bytes_alloc, avg_bytes)
        if dev > args.threshold:
            print(f"Byte deviation {dev:.2%} exceeds threshold")
            flagged = True
    if pps:
        dev = max_dev(pps, avg_pps)
        if dev > args.threshold:
            print(f"Throughput deviation {dev:.2%} exceeds threshold")
            flagged = True

    if flagged:
        sys.exit(1)


if __name__ == "__main__":
    main()
