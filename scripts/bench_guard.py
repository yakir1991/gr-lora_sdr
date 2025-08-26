#!/usr/bin/env python3
"""
Fails CI if performance regresses beyond allowed thresholds.
- Reads: bench_OUT/bench_OFF.csv and bench_ON.csv
- Optional config file: bench/targets.json
  {
    "min_pps_off": 10000,
    "min_pps_on":  10000,
    "min_ratio_on_over_off": 0.95
  }
Also accepts env overrides:
  MIN_PPS_OFF, MIN_PPS_ON, MIN_RATIO
"""
import os, sys, json, csv


def read_pps(path):
    with open(path, newline="") as f:
        r = csv.DictReader(f)
        row = next(r)
        return float(row["packets_per_sec"])


def main():
    if len(sys.argv) != 2:
        print("usage: bench_guard.py <bench_out/<timestamp>>", file=sys.stderr)
        sys.exit(2)
    folder = sys.argv[1]
    off = os.path.join(folder, "bench_OFF.csv")
    on = os.path.join(folder, "bench_ON.csv")
    if not (os.path.isfile(off) and os.path.isfile(on)):
        print(f"missing CSVs in {folder}", file=sys.stderr)
        sys.exit(3)

    # defaults
    cfg = {"min_pps_off": 0.0, "min_pps_on": 0.0, "min_ratio_on_over_off": 0.90}
    cfg_path = os.path.join("bench", "targets.json")
    if os.path.isfile(cfg_path):
        with open(cfg_path) as f:
            cfg.update(json.load(f))

    # env overrides
    cfg["min_pps_off"] = float(os.getenv("MIN_PPS_OFF", cfg["min_pps_off"]))
    cfg["min_pps_on"] = float(os.getenv("MIN_PPS_ON", cfg["min_pps_on"]))
    cfg["min_ratio_on_over_off"] = float(os.getenv("MIN_RATIO", cfg["min_ratio_on_over_off"]))

    pps_off = read_pps(off)
    pps_on = read_pps(on)
    ratio = (pps_on / pps_off) if pps_off > 0 else 0.0

    print(f"[guard] OFF={pps_off:.3f} pps; ON={pps_on:.3f} pps; ratio={ratio:.3f}")
    print(
        f"[guard] thresholds: min_pps_off={cfg['min_pps_off']}, min_pps_on={cfg['min_pps_on']}, min_ratio={cfg['min_ratio_on_over_off']}"
    )

    ok = True
    if pps_off < cfg["min_pps_off"]:
        print("[guard][FAIL] OFF below minimum")
        ok = False
    if pps_on < cfg["min_pps_on"]:
        print("[guard][FAIL] ON below minimum")
        ok = False
    if ratio < cfg["min_ratio_on_over_off"]:
        print("[guard][FAIL] ON/OFF ratio below minimum")
        ok = False

    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()

