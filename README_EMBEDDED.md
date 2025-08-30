# LoRa Lite — Embedded Optimization Plan

This document defines guardrails, build profiles, block-by-block priorities, and the verification pipeline for making LoRa Lite suitable for constrained embedded targets. It complements the main README and preserves the GNU Radio reference under `legacy_gr_lora_sdr/` untouched.

## Guardrails
- No source breakage: do not modify `legacy_gr_lora_sdr/`; all changes live under `src/` guarded by build flags.
- No dynamic allocations at runtime: every block operates on caller-provided buffers and a pre-allocated workspace.
- Full fixed-point path (Q15): enable with `-DLORA_LITE_FIXED_POINT=ON`; apply consistent scaling/saturation across modules.
- Separated profiles: keep `Release` and `RelWithDebInfo`; add a dedicated Embedded profile (size/throughput tuned, LTO, GC sections).
- Clean ARM adaptations: optional CMSIS-DSP backend for FFT/Q15 via `-DLORA_LITE_USE_CMSIS=ON` (no hard platform tie).
- Test the whole chain: Unit → Property → Vector/Golden → Full-chain compare against the GNU Radio reference.

## Memory Ownership API
To remove `malloc` from hot paths, each block exposes workspace sizing and explicit initialization:

```c
// Query required workspace size for a given configuration
size_t lora_<block>_workspace_bytes(const lora_<block>_cfg_t *cfg);

// Initialize with caller-owned memory; no allocations inside
int lora_<block>_init(lora_<block>_ctx_t      *ctx,
                      const lora_<block>_cfg_t *cfg,
                      void                    *workspace,
                      size_t                   workspace_bytes);
```

- The application allocates workspaces (static, global arena, or RTOS pool) and passes them into `*_init`.
- No `malloc`/`free` in runtime paths; initialization is idempotent and leak-free across stress loops.

Example (RX chain FFT demod workspace):

```c
lora_rx_workspace ws = {0};
size_t need = lora_rx_fft_workspace_bytes(&cfg);
static _Alignas(32) unsigned char fft_ws_buf[/* provisioned size */];
assert(sizeof fft_ws_buf >= need);
ws.fft_ws = fft_ws_buf;
ws.fft_ws_size = sizeof fft_ws_buf;
/* Now call lora_rx_chain() with caller-owned buffers and workspace */
```

## Build Profiles and Flags
- Profiles:
  - `Release` / `RelWithDebInfo`: standard host builds.
  - Embedded profile: `-Os -flto -ffunction-sections -fdata-sections -Wl,--gc-sections` plus reduced logging.
- Core flags (kept documented and consistent across modules):
  - `-DLORA_LITE_FIXED_POINT=ON` — Q15 end-to-end.
  - `-DLORA_LITE_USE_CMSIS=ON` — optional CMSIS-DSP FFT/Q15 (ARM only).
  - `-DLORA_LITE_FFT_BITREV_COPY=ON` — opt-in copy for bit-reversal, otherwise in-place.
  - `-DLORA_LITE_EMB_THROUGHPUT=ON` — micro-optimizations tuned for throughput on embedded.
  - `-DLORA_LITE_ENABLE_LOGGING=0` (default for embedded) — logging disabled; asserts only in Debug.
  - Existing optional FFT backend: `-DLORA_LITE_USE_LIQUID_FFT=ON` (host builds/CI comparisons).

Example Embedded build:

```bash
cmake -S . -B build-emb \
  -DCMAKE_BUILD_TYPE=Release \
  -DLORA_LITE_FIXED_POINT=ON \
  -DLORA_LITE_ENABLE_LOGGING=0 \
  -DLORA_LITE_EMB_THROUGHPUT=ON \
  -DCMAKE_C_FLAGS_RELEASE="-Os -flto -ffunction-sections -fdata-sections" \
  -DCMAKE_EXE_LINKER_FLAGS_RELEASE="-Wl,--gc-sections"
cmake --build build-emb -j"$(nproc)"
ctest --test-dir build-emb -V
```

## Work Plan
A hybrid approach: establish cross-cutting infrastructure, then optimize block-by-block from highest impact downward.

### Stage A — Cross-Cutting Infrastructure
- Memory API: add `*_workspace_bytes()` and `*_init(ctx, cfg, workspace, workspace_bytes)` to all blocks.
- Build/Config flags: document and wire flags consistently (float/fixed, FFT options, embedded toggles).
- Logging/Asserts: disable logs by default on embedded; keep asserts in Debug only.
- Validation: CI matrix for no-malloc and init/deinit stress (no leaks), flag combinations, and perf sanity (PPS not regressing >~2%).

### Stage B — Block Priorities and Actions
Blocks names align with current build logic (e.g., `fft`, `interleaver`, `graymap`, `header`, `payload_id`, `lora_io`, `signal_utils`, `lora_utils`).

1) FFT (`src/lora_fft.c`)
- Goals: minimize copies, per-config twiddle/index tables, efficient Q15.
- Actions: plan structure in workspace; eliminate allocations. Provide radix-2 path and optional CMSIS (`-DLORA_LITE_USE_CMSIS=ON`). Use `restrict`, 64B alignment, in-place bit-reversal (or copy-once with `LORA_LITE_FFT_BITREV_COPY`). Q15 uses saturating multiplies; on ARM use `__SSAT` where available.
- Tests: unit round-trip (FFT→iFFT≈x) in float and Q15, energy/RMS error; vectors for known signals; micro-bench (cycles/copies); chain-level PPS/BER parity against baseline.

2) De/Interleaver (`src/lora_interleaver.c`)
- Actions: precompute permutations per (SF, CR, LDRO) once per context; in-place processing; nibble LUT for bit shuffles.
- Tests: property (invertibility), vector patterns (single-bit, alternating), micro-bench, chain.

3) Gray Map / Demap (`src/lora_graymap.c`)
- Actions: Gray LUT (16 or 256 entries) to remove branches; branchless demap; fixed-point soft metrics via precomputed Q15 distance tables.
- Tests: map↔demap inversion, monotonic distances, vectors, chain.

4) Header Encode/Decode (`src/lora_header.c`)
- Actions: allocation-free parser/packer; local CRC/checksum; strict input bounds.
- Tests: light fuzzer (invalid subranges), known cases, field matching vs legacy.

5) Hamming / De-whitening / Payload ID (`src/lora_payload_id.c`)
- Actions: Hamming via `syndrome->fix` table (256 entries), branchless correction; whitening via byte-wise LFSR table (224/256 entries) instead of bit steps.
- Tests: known frames, whitening round-trip, single-error correction.

6) Signal / IO / Utils (`src/signal_utils.c`, `src/lora_io.c`, `src/lora_utils.c`)
- Actions: transparent buffer I/O; `static inline` hot helpers; remove unnecessary `memcpy`; unify Q15 scaling via a single macro.
- Tests: zero-latency, zero-alloc; stable ABI.

### Stage C — System-Wide Finishing
- Shared workspaces: reuse contiguous workspace regions across adjacent blocks to reduce RAM.
- Global read-only tables: per-SF/CR tables in a single `.rodata` unit.
- Profiling/benchmarks: export CSV (PPS, cycles, bytes_allocated) and auto-compare OFF/ON or baseline/optimized.

## Verification Pipeline
Run a short, measurable pipeline after any change.

- Build & Unit:
  ```bash
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
  cmake --build build -j"$(nproc)"
  ctest --test-dir build -V
  ```
- Property/Vector: block-specific properties and known vectors.
- Full-chain compare: run a reference pipeline that emits `bin/pcap` or bitstreams, compare bitwise/metrics vs GNU Radio reference (expected: “Full chain outputs match”).
- Bench (micro + macro): run scripts to produce `packets_per_sec`, cycles, `bytes_allocated`, compare variants (before/after; FFT backends; float/fixed), diff CSV.
- Flag matrix: at minimum cover float/fixed, FFT backends, `-Os` vs `-O3`.
- Store results: keep outputs under `bench_out/` and track trends.

## Recommended Workflow
For each block:
- Open an issue “Optimize <block> for embedded” with a checklist: API/Workspace → Fixed-point → LUTs/Branchless → Copies↓ → Tests → Bench.
- Submit a focused PR (one block per PR).
- Include a small before/after table in the PR (PPS, cycles, bytes allocated).
- If a change affects shared data structures (e.g., buffer types), update dependents immediately.

## ARM/AArch64 Bench Scripts

- Presets: see `CMakePresets.json` for `arm-embedded-release` (env `ARM_TOOLCHAIN_FILE`) and `aarch64-embedded-release` (env `AARCH64_TOOLCHAIN_FILE`). Both configure `-Os -flto`, Liquid OFF, and enable NEON/throughput guards.

- Remote FFT bench and append to CSV:
  - `scripts/run_fft_bench_remote.sh PRESET user@host:/remote/dir [tag]`
  - Copies `bench_fft` + `liblora_fft.so`, runs on target with `LD_LIBRARY_PATH=.`, fetches output, and appends FFT metrics to `bench_emb.csv` with the given tag.

- Remote full‑chain bench and append PPS:
  - `scripts/run_chain_bench_remote.sh PRESET user@host:/remote/dir [tag]`
  - Copies `bench_lora_chain` and build‑tree `.so` libs, runs on target, and appends `bench_results.csv` to `bench_emb.csv`.

- Local embedded‑profile chain bench:
  - `scripts/run_chain_bench_local.sh`
  - Builds with `-Os -flto`, runs the chain bench locally, and appends results to `bench_emb.csv`.

## ARM/CMSIS Notes
- When targeting ARM, `-DLORA_LITE_USE_CMSIS=ON` enables CMSIS-DSP FFT/Q15. Keep a portable fallback (radix-2) to avoid hard platform dependencies.
- Prefer in-place transforms, `restrict` pointers, and alignment-friendly layouts for DMA-friendly pipelines.

## Reference Preservation
The original GNU Radio implementation is retained under `legacy_gr_lora_sdr/` for full-chain comparisons and as a correctness baseline. LoRa Lite remains a modular sandbox; legacy code is not modified.
