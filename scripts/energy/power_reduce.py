#!/usr/bin/env python3
import sys, csv, re


def parse_energy(csv_path):
    ts = []; p = []
    with open(csv_path, newline="") as f:
        r = csv.DictReader(f)
        for row in r:
            ts.append(int(row["time_ms"]))
            p.append(float(row["power_mw"]))
    if len(ts) < 2:
        return 0.0, 0.0, 0.0
    dur_s = (ts[-1]-ts[0])/1000.0
    # trapezoid integration (power in mW -> W)
    energy_j = 0.0
    for i in range(1, len(ts)):
        dt = (ts[i]-ts[i-1])/1000.0
        energy_j += 0.5*((p[i]+p[i-1]) / 1e3)*dt
    avg_p_mw = (sum(p)/len(p))
    return avg_p_mw, energy_j, dur_s


def parse_pps(stdout_log):
    rgx = re.compile(r"packets_per_sec=([0-9]+(?:\.[0-9]+)?)")
    vals = []
    with open(stdout_log, "r") as f:
        for line in f:
            m = rgx.search(line)
            if m: vals.append(float(m.group(1)))
    if not vals:
        return 0.0
    return sum(vals)/len(vals)


def main():
    if len(sys.argv) != 4:
        print("usage: power_reduce.py <energy.csv> <bench_stdout.log> <summary_out.csv>", file=sys.stderr)
        sys.exit(2)
    p_mw, e_j, dur = parse_energy(sys.argv[1])
    pps = parse_pps(sys.argv[2])
    e_per_packet_mJ = (e_j*1000.0) / (pps*dur) if (pps>0 and dur>0) else 0.0
    with open(sys.argv[3], "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["avg_power_mw","energy_j","duration_s","pps_avg","energy_per_packet_mJ"])
        w.writerow([f"{p_mw:.3f}", f"{e_j:.6f}", f"{dur:.3f}", f"{pps:.3f}", f"{e_per_packet_mJ:.6f}"])
    print(f"[reduce] avg_power_mw={p_mw:.1f} energy_j={e_j:.4f} duration_s={dur:.2f} pps_avg={pps:.1f} e/packet={e_per_packet_mJ:.3f} mJ")


if __name__ == "__main__":
    main()
