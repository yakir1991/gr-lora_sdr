# LoRa Lite — FFT Experiments (Host & Embedded)

**Date:** 2025-08-27  
**Host:** WSL2 Ubuntu 22.04, VS Code (Remote WSL)  
**CPU:** Intel® Core™ i9-12900K (24 cores)  
**Compiler:** GCC 11.4.0

## Methodology

- Functional validation via `ctest` (unit, golden, end-to-end).
- Performance via `bench_lora_chain` (reports `cycles`, `peak_bytes`, `packets_per_sec`).
- FFT variants:
  - **FFT=OFF** — internal/KISS-style FFT (fixed-friendly).
  - **FFT=ON** — Liquid-DSP FFT.
- Build profiles:
  - **Release (host)** — default CMake flags.
  - **Embedded (EMBED=1)** — static link, `-Os`, GC-sections, LTO (where applicable).
  - **Fixed-Point** — `-DLORA_LITE_FIXED_POINT=ON`.
- For more stable numbers we also pin to a single core (`taskset -c 0`) and take multiple runs (median).

### Reproduce

```bash
# Full test suite (Release)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build -j"$(nproc)"
ctest --test-dir build -V
ctest --test-dir build -R bench_lora_chain -V

# Full matrix + summary (float + fixed, with embedded profile)
INCLUDE_FIXED=1 EMBED=1 LOG=1 ./scripts/full_test_matrix.sh
LATEST_SUM_DIR=$(ls -1d bench_out/emb-matrix-* | tail -n1)
sed -n '1,200p' "$LATEST_SUM_DIR/SUMMARY.md"
```

---

## Results

### A) Host (Release), Float vs Fixed, FFT OFF/ON
(from a matrix run with **EMBED=0**)

| Mode   | FFT | packets_per_sec |
|--------|-----|----------------:|
| Float  | OFF | 9,113.460 |
| Float  | ON  | **9,191.816** *(+0.86% vs OFF)* |
| Fixed  | OFF | **9,794.881** |
| Fixed  | ON  | 8,312.481 *(-15.13% vs OFF)* |

**Takeaway:** on host float builds, Liquid-DSP is slightly faster; on fixed-point it is significantly slower.

---

### B) Embedded profile (EMBED=1), Float vs Fixed, FFT OFF/ON
(from a matrix run with **EMBED=1**)

| Mode   | FFT | packets_per_sec |
|--------|-----|----------------:|
| Float  | OFF | 8,333.148 |
| Float  | ON  | **8,442.264** *(+1.31% vs OFF)* |
| Fixed  | OFF | **11,097.650** |
| Fixed  | ON  | 8,165.037 *(ON/OFF = 0.7357)* |

**Takeaway:** for the embedded profile (fixed-point), **FFT=OFF** clearly wins.

---

### C) Pinned, apples-to-apples (single core)

| Build                  | FFT | cycles (×10⁶) | pps        |
|------------------------|-----|---------------:|-----------:|
| Fixed (embedded-like)  | OFF | **10.421**     | **9,595.991** |
| Float (host)           | ON  | 10.823         | 9,239.332  |

**Takeaway:** even on the host, fixed+OFF edges out float+ON (~+3.9%) and uses fewer cycles.

---

### D) Clean host benchmark

- Not pinned: **8,172.726 pps**  
- Pinned (`taskset -c 0`): **9,895.160 pps**

**Takeaway:** pinning improves stability and throughput (~+21%). For benchmarking: pin CPU, run ≥5 times, report **median**.

---

### E) Micro metric (demod)

`test_lora_mod_fft` (Fixed, FFT=OFF):

- Demod workspace: **12,288 B → 8,192 B** (≈ −33%)
- Demod time: **31.00 µs**
- `peak_bytes` near **5.8 KB** across runs

---

### F) FFT Demod Block — correctness + micro-bench

- Correctness: `test_fft_demod_correctness` passes on both host and embedded builds (symbol 0 recovered for ideal upchirps).
- Micro-benchmark (`bench_demod`, sf7/os8, nsym=128):
  - Host (Release): float 14.551 µs/sym; fixed 30.203 µs/sym.
  - Embedded profile (after Q15 fast-path): float ≈9.61 µs/sym; fixed ≈10.25 µs/sym.
- Workspace: 12,288 B → 16,896 B (added Q15 downchirp + bins).

Notes:
- Q15 fast-path (CFO=0): accumulates in Q15 per bin, converts only `n_bins` to float for FFT (previously converted `sps`).
- In embedded profile this roughly halves the demod time for CFO=0; memory increases by ~4.5 KB.
- LTO/GC-sections: added `q15_to_cf.c` to `lora_fft_demod` to avoid missing symbol.

Next up: consider precomputing a Q15 downchirp table to avoid per-symbol conversion in the fixed path and measure impact.

---

### G) Whitening — throughput micro-bench

- Impl: removed modulo per-byte; process in chunks bounded by whitening period (255), avoiding `%` in hot loop. Added 64-bit XOR within each chunk.
- Micro-benchmark (`bench_whitening`, 8 KB buffer, 2000 iters):
  - Host: ~314 MB/s
  - Embedded profile: ~312 MB/s

Notes: whitening is now memory-bandwidth bound; further gains likely require vector intrinsics (e.g., NEON/SSE) which we can add per-arch later.

---

### H) CRC — table-driven implementation + bench

- Impl: replaced per-bit loop with table-driven CRC-16-CCITT (poly 0x1021, init 0x0000), preserving legacy post-XOR and nibble extraction.
- Correctness: existing `test_lora_add_crc` passes (host + embedded).
- Micro-benchmark (`bench_crc`, 512 B payload, 200k iters):
  - Host: ~64.8 MB/s
  - Embedded profile: ~64.9 MB/s

Notes: current CRC throughput is adequate relative to overall chain cost; further gains possible with prefetching or unrolling, but not currently a bottleneck.

---

### I) FFT Demod — CFO path (Q15) micro-bench

- Impl: CFO compensation now runs in Q15 as well (per-sample Q15 phasor), keeping the fixed-point fast-path when CFO ≠ 0.
- Micro-benchmark (`bench_demod_cfo`, cfo=100 Hz, sf7/os8, nsym=128):
  - Host (Release): ≈30.57 µs/sym (ok=1)
  - Embedded: ≈21.84 µs/sym (ok=1)

Notes: accumulation switched to 32-bit with saturating store to avoid Q15 overflow; phase continuity fixed across bins. CFO correctness verified by rotating input chips and recovering symbol 0.

---

### J) Modulation — upchirp recurrence

- Impl: replaced per-sample `cexpf` in `lora_build_upchirp` with a second-order recurrence using a complex step (`r`) and constant step-of-step (`step_r`), with a single adjustment at the fold.
- Micro-benchmark (`bench_mod`, sf7/os8, nsym=2048):
  - Host: ≈30.70 µs/sym
  - Embedded: ≈30.85 µs/sym

Notes: this eliminates thousands of `cexpf` calls per packet; remaining cost is simple complex multiplies. Numbers shown are single-run; for rigorous comparison, pin a core and take medians across runs.

---

### K) Internal FFT — bitrev copy + small unroll

- Impl: avoid a separate bit-reverse pass by copying input into the work buffer in bit-reversed order; add a tiny unroll (×2) in the butterfly inner loop.
- Micro-benchmark (`bench_fft`, us per exec):
  - Host: sf7 4.98 → 4.98, sf10 56.18 → 56.18 (within noise)
  - Embedded: sf7 ≈4.70, sf10 ≈53.21

Notes: For these sizes the wins are modest and within run-to-run variance; however, the change reduces memory traffic and scales better as `n` grows. Further gains would come from stage-wise twiddle layout and SIMD. Keeping changes minimal for stability.


## Conclusions

1. **Embedded default = Fixed + FFT=OFF.**  
   Faster and more memory-friendly than Liquid in this profile.
2. **Liquid-DSP under fixed-point** is penalized by Q15↔float conversions and loses vectorization under `-Os`/static; in practice it is slower.
3. **Host float (dev)**: Liquid provides a small gain (~1%); acceptable as default for desktop development.
4. **Benchmark hygiene**: pin a core, run multiple iterations, use **median**.

---

## Next Steps (Plan)

### 1) Lock the embedded profile
- Embedded preset should enforce: `LORA_LITE_FIXED_POINT=ON`, `LORA_LITE_USE_LIQUID_FFT=OFF`.
- Emit a **configure-time warning** when `FIXED=ON && FFT=ON` (“usually slower on embedded”).
- CI guard for fixed: treat **FFT=OFF as the baseline**; check `min_pps_off`, and (optionally) cap `ON/OFF` ratio (e.g., `≤ 0.90`) instead of expecting `> 1`.

### 2) Micro-optimizations (no API change, no runtime allocs)
- **Internal FFT (KISS)**
  - Static/once-only twiddles (no `sin/cos` in hot path).
  - `restrict` qualifiers on I/O pointers; 16–32B alignment; small butterfly unroll.
  - Target: **+2–4%** pps.
- **Whitening / Interleaver**
  - Batch on `uint64_t` (x86); branchless loops; prep a NEON path for ARM later.
  - Target: **+1–3%** pps.
- **Fixed-math cleanup**
  - Remove redundant checks/branches in hot loops; keep current Q15 scaling (tests pass).
  - Target: **≈+1%**.

### 3) Measurement robustness
- Add pinning and N-run median into `full_test_matrix.sh`.
- Record `median_pps` and `stdev` per build in `SUMMARY.md`.

### 4) Optional: alternative FFT for ARM
- Prototype a **CMSIS-DSP** Q15 FFT backend (`arm_cfft_q15` / `arm_rfft_q15`) for real ARM hardware (not QEMU).
- Add as an optional CMake backend (`LORA_LITE_FFT_BACKEND=cmsis`), build only on ARM cross builds.

---

## Action Items

- [ ] Embedded preset enforces `FIXED=ON, FFT=OFF` (+ warning if combined with Liquid).  
- [ ] CI guard thresholds updated for fixed (baseline OFF).  
- [ ] Pinning + median added to the matrix script; `median_pps` persisted to summary.  
- [ ] Implement internal FFT micro-tweaks and re-benchmark (≥5 runs, pinned, median).  
- [ ] (Optional) Prototype CMSIS-DSP backend on ARM.

> Note: absolute numbers are host-specific; the *relative* trends (Fixed+OFF > Fixed+ON, Float: ON ≳ OFF) were consistent across our runs.

---

## Roadmap — Further Optimizations (Prioritized)

- TX Q15 Path: generate chips directly in Q15 for fixed builds to remove float round-trips. Low risk, small–moderate gain.
- CFO Phasor Fully Q15: keep the CFO phasor as Q1.15 and iterate via Q15 complex multiply (no per-sample float phasor). Moderate gain; tune precision.
- Interleaver SIMD: 16B SSE2/NEON for bit moves and parity via popcount/LUT. Small gain; low risk.
- FFT Algorithm: add radix-4 or split-radix for n divisible by 4; typically trims ~10–20% vs pure radix-2. Medium complexity.
- Twiddle Layout: stage-wise twiddle blocks for better locality + slightly larger inner unrolls gated by size. Small–moderate gain.
- Memory/Layout: precompute bit-reversal indices for common sizes; ensure 64B alignment on large buffers; apply `restrict` consistently. Small gain.
- ARM Backend: prototype CMSIS-DSP FFT (`arm_cfft_q15`) for ARM targets; may deliver moderate–large wins on real hardware.
- Frequency-Domain CFO: compensate CFO post-FFT (bin-wise phase slope) for small/slow CFOs. Scenario-dependent; validate accuracy.
- Batch Symbols: dechirp multiple symbols then batched FFTs to improve locality; requires workspace planning. Small–moderate gain.
- Toolchain: PGO builds and arch-specific flags (e.g., cortex/NEON, `-ffp-contract=fast`) to harvest additional percent-level gains.

Guardrails and Benchmarks
- Keep `pps_guard` in CI with profile-specific thresholds; record median over multiple runs.
- When adding new paths (NEON/SSE/CMSIS), gate by CMake options and keep scalar fallbacks tested.
