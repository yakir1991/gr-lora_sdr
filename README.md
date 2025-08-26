# lora_lite

See [doc/README.md](doc/README.md) for full project documentation.

The original GNU Radio-based implementation is preserved in [legacy_gr_lora_sdr/](legacy_gr_lora_sdr/) and remains unmodified for reference.

## Benchmark sweep

Run `./scripts/sweep_bench.sh` to build and execute the benchmark across multiple
spreading factors, coding rates, LDRO settings, fixed-point modes, and logging options.
Results accumulate in `results/host_sweep.csv` with columns:

```
sf,cr,ldro,fixed,logging,cycles,bytes_allocated,packets_per_sec
```

Analyze the CSV to compare performance across configurations.

## AArch64 QEMU benchmark

Prerequisites:

- `aarch64-linux-gnu-gcc` cross-compiler
- `qemu-aarch64` user emulator

Build and run the benchmark under QEMU:

```sh
./scripts/run_qemu_aarch64.sh
```

The script saves metrics to `results/arm_qemu.csv`. Compare them with host
measurements using:

```sh
python scripts/analyze_bench.py results/host.csv results/arm_qemu.csv
```
