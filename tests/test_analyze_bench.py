import subprocess
import sys
from pathlib import Path


def _run(threshold: float) -> subprocess.CompletedProcess:
    repo_root = Path(__file__).resolve().parents[1]
    script = repo_root / "scripts" / "analyze_bench.py"
    data = Path(__file__).parent / "data"
    return subprocess.run(
        [sys.executable, str(script), str(data / "base.csv"), str(data / "comp.csv"), "--threshold", str(threshold)],
        capture_output=True,
        text=True,
    )


def test_analyze_bench_threshold_pass():
    res = _run(0.2)
    assert res.returncode == 0
    assert "cycles" in res.stdout
    assert "\u2713" in res.stdout


def test_analyze_bench_threshold_fail():
    res = _run(0.05)
    assert res.returncode == 1
    assert "\u26A0" in res.stdout

