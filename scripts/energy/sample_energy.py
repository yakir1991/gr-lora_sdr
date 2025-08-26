#!/usr/bin/env python3
import argparse, glob, os, sys, time, subprocess, csv


def read_int(path):
    with open(path, "r") as f:
        return int(f.read().strip())


def find_power_file():
    # Highest priority: explicit env
    pf = os.getenv("POWER_FILE")
    if pf and os.path.isfile(pf):
        return ("power_uW", pf, None, None)

    # /sys/class/power_supply/*/power_now (µW)
    for p in glob.glob("/sys/class/power_supply/*/power_now"):
        if os.path.isfile(p):
            return ("power_uW", p, None, None)

    # hwmon power*_input (µW)
    for p in glob.glob("/sys/class/hwmon/hwmon*/power*_input"):
        if os.path.isfile(p):
            return ("power_uW", p, None, None)

    # power_supply voltage_now (µV) + current_now (µA)
    for base in glob.glob("/sys/class/power_supply/*"):
        v = os.path.join(base, "voltage_now")
        c = os.path.join(base, "current_now")
        if os.path.isfile(v) and os.path.isfile(c):
            return ("v_uV_i_uA", None, v, c)

    # Allow explicit VOLTAGE_FILE/CURRENT_FILE via env
    vf = os.getenv("VOLTAGE_FILE"); cf = os.getenv("CURRENT_FILE")
    if vf and cf and os.path.isfile(vf) and os.path.isfile(cf):
        return ("v_uV_i_uA", None, vf, cf)

    return None


def read_power_w(kind, power_path, v_path, c_path):
    if kind == "power_uW":
        return read_int(power_path) / 1e6  # µW -> W
    elif kind == "v_uV_i_uA":
        v_uV = read_int(v_path)
        i_uA = read_int(c_path)
        return (v_uV * 1e-6) * (i_uA * 1e-6)  # (V)*(A)=W
    else:
        raise RuntimeError("unknown sensor kind")


def main():
    ap = argparse.ArgumentParser(description="Sample power while running a command")
    ap.add_argument("--out", required=True, help="CSV output (time_ms,power_mw)")
    ap.add_argument("--interval-ms", type=int, default=50)
    ap.add_argument("--sensor", default="auto", choices=["auto", "sysfs"])
    ap.add_argument("--quiet", action="store_true")
    ap.add_argument("--stop-after", type=float, default=0.0,
                    help="Optional: stop after N seconds (caller should also control cmd duration).")
    ap.add_argument("cmd", nargs=argparse.REMAINDER, help="-- command to run ...")
    args = ap.parse_args()

    if not args.cmd or args.cmd[0] != "--":
        print("usage: sample_energy.py --out <csv> [--interval-ms 50] -- <command>", file=sys.stderr)
        sys.exit(2)
    cmd = args.cmd[1:]

    found = find_power_file()
    if not found:
        print("[energy] no power/voltage/current sysfs sensor found. Set POWER_FILE or VOLTAGE_FILE/CURRENT_FILE.", file=sys.stderr)
        sys.exit(4)
    kind, p_path, v_path, c_path = found
    if not args.quiet:
        print(f"[energy] using sensor kind={kind} "
              f"{p_path or ''} {v_path or ''} {c_path or ''}", file=sys.stderr)

    proc = subprocess.Popen(cmd)
    t0 = time.time()
    next_deadline = t0 + args.stop_after if args.stop_after > 0 else None

    with open(args.out, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["time_ms", "power_mw"])
        while True:
            now = time.time()
            try:
                pw = read_power_w(kind, p_path, v_path, c_path)
            except Exception as e:
                print(f"[energy] read error: {e}", file=sys.stderr)
                break
            w.writerow([int((now - t0)*1000), pw*1e3])
            f.flush()
            # exit conditions
            if proc.poll() is not None:
                if next_deadline and now < next_deadline:
                    # keep sampling until stop-after
                    pass
                else:
                    break
            if next_deadline and now >= next_deadline:
                # do not kill proc here; wrapper controls its lifetime
                if proc.poll() is not None:
                    break
            time.sleep(max(0.0, args.interval_ms/1000.0))
    # return underlying exit code if finished
    rc = proc.poll()
    sys.exit(0 if rc is None else rc)


if __name__ == "__main__":
    main()
