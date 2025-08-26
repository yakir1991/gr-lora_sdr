#!/usr/bin/env python3
"""Compare benchmark metrics across two CSV files.

Each CSV is expected to have the columns ``metric`` and ``value``.

Examples
--------
Run with the default lenient threshold::

    python scripts/analyze_bench.py base.csv comp.csv

Enforce a maximum 10% deviation::

    python scripts/analyze_bench.py base.csv comp.csv --threshold 0.1 --strict
"""

from __future__ import annotations

import argparse
import csv
import sys
from typing import Dict


def _read_metrics(path: str) -> Dict[str, float]:
    """Load metric values from *path*.

    Parameters
    ----------
    path: str
        CSV file containing ``metric`` and ``value`` columns.
    """

    with open(path, newline="") as f:
        first = f.readline().strip().split(",")
        f.seek(0)
        if {"metric", "value"} <= set(first):
            reader = csv.DictReader(f)
            return {row["metric"]: float(row["value"]) for row in reader}
        else:
            reader = csv.reader(f)
            header = next(reader)
            values = next(reader)
            return {h: float(v) for h, v in zip(header, values)}


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Compare benchmark CSV metrics",
    )
    parser.add_argument("base_csv", help="Baseline metrics CSV")
    parser.add_argument("comp_csv", help="Comparison metrics CSV")
    parser.add_argument(
        "--threshold",
        type=float,
        default=0.05,
        help="Allowed fractional difference before flagging",
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Exit with status 1 if any metric exceeds the threshold",
    )
    args = parser.parse_args()

    base = _read_metrics(args.base_csv)
    comp = _read_metrics(args.comp_csv)

    flagged = False
    metrics = sorted(base.keys() & comp.keys())
    for name in metrics:
        a = base[name]
        b = comp[name]
        if a == 0:
            delta = float("inf") if b != 0 else 0.0
        else:
            delta = (b - a) / a
        symbol = "\u2713" if abs(delta) <= args.threshold else "\u26A0"
        print(f"{name}: {a:g} \u2192 {b:g} \u0394={delta:+.2%} [{symbol}]")
        if abs(delta) > args.threshold:
            flagged = True

    if flagged and args.strict:
        sys.exit(1)


if __name__ == "__main__":
    main()

