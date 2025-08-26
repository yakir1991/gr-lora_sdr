import subprocess
import sys
from pathlib import Path


def _run(threshold: float, strict: bool) -> subprocess.CompletedProcess:
    repo_root = Path(__file__).resolve().parents[1]
    script = repo_root / "scripts" / "analyze_bench.py"
    data = Path(__file__).parent / "data"
    cmd = [
        sys.executable,
        str(script),
        str(data / "base.csv"),
        str(data / "comp.csv"),
        "--threshold",
        str(threshold),
    ]
    if strict:
        cmd.append("--strict")
    return subprocess.run(cmd, capture_output=True, text=True)


def test_analyze_bench_threshold_pass():
    res = _run(0.2, True)
    assert res.returncode == 0
    assert "cycles" in res.stdout
    assert "\u2713" in res.stdout


def test_analyze_bench_threshold_fail():
    res = _run(0.05, True)
    assert res.returncode == 1
    assert "\u26A0" in res.stdout


def test_analyze_bench_default_non_strict():
    res = _run(0.05, False)
    assert res.returncode == 0
    assert "\u26A0" in res.stdout

