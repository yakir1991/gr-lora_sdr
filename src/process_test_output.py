#!/usr/bin/env python3
"""Summarize simple test output logs.

The script expects a text file where each line contains a test name
followed by either "PASS" or "FAIL".  It reports the total number of
passing and failing tests and prints a short summary.
"""

from __future__ import annotations
import sys


def parse_lines(lines):
    """Parse iterable of lines and count PASS/FAIL occurrences."""
    passed = failed = 0
    entries = []
    for raw in lines:
        line = raw.strip()
        if not line:
            continue
        entries.append(line)
        upper = line.upper()
        if "PASS" in upper:
            passed += 1
        elif "FAIL" in upper:
            failed += 1
    return passed, failed, entries


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        print(f"Usage: {argv[0]} <test_output_file>")
        return 1
    path = argv[1]
    try:
        with open(path, "r", encoding="utf-8") as handle:
            passed, failed, entries = parse_lines(handle)
    except OSError as exc:
        print(f"Could not read '{path}': {exc}")
        return 1

    print("Test Summary")
    print("============")
    for entry in entries:
        print(entry)
    print()
    print(f"Passed: {passed}")
    print(f"Failed: {failed}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
