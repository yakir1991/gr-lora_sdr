#!/usr/bin/env python3
"""Compare benchmark metrics across two CSV files.

Each CSV is expected to have the columns ``metric`` and ``value``.

Examples
--------
Run with default 5% threshold::

    python scripts/analyze_bench.py base.csv comp.csv

Specify a custom threshold::

    python scripts/analyze_bench.py base.csv comp.csv --threshold 0.1
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
        reader = csv.DictReader(f)
        return {row["metric"]: float(row["value"]) for row in reader}


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

    if flagged:
        sys.exit(1)


if __name__ == "__main__":
    main()

