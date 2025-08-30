# Embedded Optimization Notes — Stage A kickoff

This document tracks concrete changes made while implementing the embedded guardrails and the first cross-cutting steps.

## Summary
- Added caller-owned workspace API for RX chain FFT demod and removed runtime allocations from `lora_rx_chain` when a workspace is provided.
- Introduced helper `lora_rx_fft_workspace_bytes(const lora_chain_cfg*)` to pre-size workspace buffers.
- Kept host/tests compatibility: when `LORA_LITE_NO_MALLOC` is not defined, `lora_rx_chain` falls back to allocating the FFT workspace on demand to keep existing tests green.
- CMake include paths adjusted so RX/TX chain can include public headers.
- Documentation: Added `README_EMBEDDED.md` and an example snippet for preallocating and attaching FFT workspace.

## Code Changes
- `src/lora_chain.h`
  - Includes `lora_fft_demod.h` directly.
  - Adds `lora_rx_fft_workspace_bytes(const lora_chain_cfg*)` declaration.
  - Modifies `lora_rx_workspace` to embed `fft_ctx` (no pointer malloc) and track provided workspace (`fft_ws`, `fft_ws_size`, `fft_inited`).
- `src/lora_rx_chain.c`
  - Uses provided `ws->fft_ws` for demod workspace; initializes `fft_ctx` over caller memory.
  - If `LORA_LITE_NO_MALLOC` is defined and workspace is missing/too small, returns `LORA_ERR_BUFFER_TOO_SMALL`.
  - Otherwise (tests/host), allocates on demand to preserve legacy usage.
  - Adds `lora_rx_fft_workspace_bytes()` helper (simple wrapper over `lora_fft_workspace_bytes()`).
- `src/CMakeLists.txt`
  - Ensures chain targets also see headers under `include/`.

## Usage Example (no malloc)
```c
lora_chain_cfg cfg = { .sf = 8, .bw = 125000, .samp_rate = 125000 };
lora_rx_workspace ws = {0};
size_t need = lora_rx_fft_workspace_bytes(&cfg);
static _Alignas(32) unsigned char fft_ws_buf[/* provisioned size */ 16384];
assert(sizeof fft_ws_buf >= need);
ws.fft_ws = fft_ws_buf;
ws.fft_ws_size = sizeof fft_ws_buf;

lora_status st = lora_rx_chain(chips, nchips, payload, sizeof payload, &out_len, &cfg, &ws);
```

Define `-DLORA_LITE_NO_MALLOC=ON` to enforce no-malloc at compile time (RX will fail if workspace is absent/undersized).

## Tests
- Rebuilt and ran focused chain/integration tests: all pass on host builds.
- Full suite remains green.
### Validation — FFT block (host)
- Round-trip (fwd→inv) across SF7–SF12 passed with tight error bounds:
  - max_err≈3.21e-7, rms≈1.08e-7 (sf=12).
- Micro-bench `bench_fft` (host, Liquid backend in this build):
  - sf7: ~4.52 µs, sf8: ~10.33 µs, sf9: ~23.17 µs,
  - sf10: ~50.17 µs, sf11: ~109.26 µs, sf12: ~236.78 µs.
  - Note: Absolute numbers depend on backend and host; compare trends before/after bit-reversal speedup.

### FFT Improvement Summary (host)
- Baseline (Liquid backend) vs optimized internal FFT (radix‑2):
  - sf7: 4.523 → 4.382 µs (+3.1%)
  - sf8: 10.329 → 9.657 µs (+6.5%)
  - sf9: 23.170 → 21.381 µs (+7.7%)
  - sf10: 50.173 → 47.060 µs (+6.2%)
  - sf11: 109.256 → 103.385 µs (+5.4%)
  - sf12: 236.776 → 231.232 µs (+2.3%)
- Changes contributing: faster bit‑reversal copy, alignment hints, copy elision, and tightened inner loops.

### Radix-4 vs Radix-2 (host, kiss backend)
- Build flags: `-DLORA_LITE_USE_LIQUID_FFT=OFF`, compare `LORA_LITE_FFT_RADIX4=OFF` vs `ON`.
- Results (µs per FFT):
  - sf7: r2=4.382, r4=4.268, Δ=+0.114 (+2.60%)
  - sf8: r2=9.657, r4=9.626, Δ=+0.031 (+0.32%)
  - sf9: r2=21.381, r4=22.069, Δ=−0.688 (−3.22%)
  - sf10: r2=47.060, r4=47.130, Δ=−0.070 (−0.15%)
  - sf11: r2=103.385, r4=103.551, Δ=−0.166 (−0.16%)
  - sf12: r2=231.232, r4=225.603, Δ=+5.629 (+2.43%)
- Takeaways (first pass): results vary by SF; we keep the flag optional. Default remains radix‑2 for stability; measure on target when enabling.

### Twiddle Recurrence (guarded)
- Implemented recurrence update of twiddles for radix‑2 stages behind `LORA_LITE_TWIDDLE_RECURRENCE` (OFF by default for numerical parity with baseline).
- With the guard OFF, round‑trip tests match baseline error bounds and pass.
- Host bench (kiss backend, radix‑2): r2 vs recurrence ON
  - sf7: 4.848 → 4.506 (Δ=+0.342 µs, +7.05%)
  - sf8: 9.953 → 10.290 (Δ=−0.337 µs, −3.39%)
  - sf9: 21.971 → 22.610 (Δ=−0.639 µs, −2.91%)
  - sf10: 48.662 → 49.746 (Δ=−1.084 µs, −2.23%)
  - sf11: 106.777 → 110.766 (Δ=−3.989 µs, −3.74%)
  - sf12: 233.435 → 240.266 (Δ=−6.831 µs, −2.93%)
- Takeaway: small win at SF7, regressions at higher SFs on this host. Keep OFF by default on host; consider enabling on embedded targets only after on‑target profiling (compiler/arch may tilt results).

### Embedded-profile bench (host, -Os -flto, kiss backend)
- Flags: `-Os -flto -ffunction-sections -fdata-sections -Wl,--gc-sections`, Liquid OFF.
- Results (µs per FFT): radix-2 vs radix-4 vs radix-2+recurrence
  - sf7: r2=4.497, r4=4.999, rec=4.494 (rec +0.07%, r4 −11.16%)
  - sf8: r2=11.003, r4=10.847, rec=10.165 (rec +7.62%, r4 +1.42%)
  - sf9: r2=22.768, r4=23.943, rec=23.643 (rec −3.84%, r4 −5.16%)
  - sf10: r2=51.104, r4=52.905, rec=51.122 (rec −0.04%, r4 −3.52%)
  - sf11: r2=110.984, r4=113.325, rec=112.939 (rec −1.76%, r4 −2.11%)
  - sf12: r2=235.985, r4=255.603, rec=244.537 (rec −3.62%, r4 −8.31%)
- Takeaways: under size/throughput flags, radix‑4 regresses on this host; recurrence is mixed (notably helps at SF8). Keep both flags optional; choose per‑target after profiling. Default remains radix‑2 without recurrence.

## Stage B — Updates (Block‑Level)

### RX Workspace Coalescing
- Consolidated `whitened[]`, `payload_crc[]`, and `tmp[]` into a single `bytes[]` buffer in `lora_rx_workspace`.
- In‑place dewhitening; CRC computed by zeroing trailing two bytes in‑place (restored after), avoiding extra copies.
- Files: `src/lora_chain.h`, `src/lora_rx_chain.c`.
- Chain tests pass: `test_lora_chain`, `test_end_to_end_file`, `test_sync_end_to_end`.

### TX Workspace Coalescing
- Removed separate `whitened[]` buffer; whiten now happens in-place in `ws->buf` and symbols are emitted from it.
- Files: `src/lora_chain.h`, `src/lora_tx_chain.c`.
- No behavior change; reduces TX RAM usage by `LORA_MAX_PAYLOAD_LEN+2` bytes.

### Demod Copy Reduction
- Scan FFT spectrum directly from `fft.work` to avoid final memcpy.
- File: `src/lora_fft_demod.c`. `test_fft_demod_correctness` and chain tests pass.

### Hamming — Branchless Syndrome Fix
- Replaced switch with 8‑entry syndrome→fix table (CR≥3). File: `src/lora_hamming.c`. Tests pass.

### Whitening — 8‑Byte LUT (opt‑in)
- Added `lora_whiten_lut8`/`lora_whiten_next8`; guard `LORA_LITE_WHITEN_LUT8`.
- Near parity with SIMD on host (seq ~322.46 MB/s vs LUT8 ~318.80 MB/s); keep OFF on host.
- Added static rodata variant `src/whiten_lut8_static.c` behind `-DLORA_LITE_WHITEN_LUT8_STATIC=ON`.

### Gray Map/Demap — Micro‑bench
- Added `bench_graymap` (Msym/s): sf7 296.54 / 299.85; sf8 293.50 / 306.03; sf9 307.18 / 302.67; sf10 301.26 / 304.33; sf11 304.87 / 297.83; sf12 296.46 / 295.42.
- CSV updated under tag `fft_target,graymap_bench`.

### Demod — Micro‑bench (no‑copy FFT scan)
- Added `bench_demod_copy` to measure demod throughput post copy-elision.
- Results (Msym/s, host): sf7 0.190; sf8 0.097; sf9 0.042; sf10 0.020; sf11 0.009; sf12 0.004.
- Results (Msym/s, embedded profile -Os -flto): sf7 0.212; sf8 0.094; sf9 0.041; sf10 0.019; sf11 0.009; sf12 0.004.
- CSV: appended under `fft_target,demod_bench` and `fft_target,demod_bench_emb`.

#### Demod (embedded profile) — FFT flags deltas
- Built three variants and ran `bench_demod_copy`:
  - Base (radix‑2), Radix‑4 ON, Recurrence ON.
- Results (Msym/s; Δ vs base):
  - sf7: base 0.212, r4 −2.36%, rec −6.13%
  - sf8: base 0.094, r4 +1.06%, rec +1.06%
  - sf9..12: effectively neutral (0.00%).
- Takeaway: no clear gain from Radix‑4/Recurrence in the demod path on this host embedded profile; keep both optional and decide per target.

## Tooling & Presets
- CMake presets: `arm-embedded-release` (env `ARM_TOOLCHAIN_FILE`), `aarch64-embedded-release` (env `AARCH64_TOOLCHAIN_FILE`).
- Preset defaults (embedded): enable `LORA_LITE_WHITEN_LUT8=ON` and `LORA_LITE_WHITEN_LUT8_STATIC=ON` by default to favor static rodata over init time.
- Scripts: `scripts/run_fft_bench_remote.sh`, `scripts/run_chain_bench_remote.sh`, `scripts/run_chain_bench_local.sh` to build/copy/run benches and append results to `bench_emb.csv`.

## ARM/NEON Hooks
- Guarded NEON complex mul in hot radix‑2 butterflies and prefetch hints. Guards: `LORA_LITE_NEON`, `LORA_LITE_EMB_THROUGHPUT`.
 - Action: When an ARM toolchain is available, run `bench_demod_copy` under the embedded presets to capture NEON gains; record under a new CSV tag (e.g., `demod_bench_arm_neon`).

## RO Tables Consolidation
- Current: Gray LUT (`src/lora_gray_lut.h`) and CRC table (`src/lora_crc_table.h`) in `.rodata`.
- Whitening LUT8: static rodata variant available; otherwise built at init for zero‑flash variants.
- Added consolidation TU `src/ro_tables.c` to anchor large RO tables under one object for icache locality and predictable LTO/GC behavior.
- Plan: consolidate per‑SF/CR read‑only tables into a single translation unit for locality; gate optional large LUTs with flags to balance flash vs cycles.

## Memory Deltas (Workspace)
- RX: replaced three N‑byte buffers (`whitened`, `payload_crc`, `tmp`) with one N‑byte `bytes` buffer — saves 2×`LORA_MAX_NSYM` bytes.
- TX: removed `whitened` (`LORA_MAX_PAYLOAD_LEN+2` bytes) by whitening in‑place — saves `LORA_MAX_PAYLOAD_LEN+2` bytes.
- Flash (RO): adding static whitening LUT8 increases flash by ~2304 bytes (256×8‑byte masks + 256×1‑byte next‑state), removes runtime init cost.

## Header Block — Micro‑bench
- Added `bench_header` to measure encode/decode throughput for the header block.
- Results (host O3): encode ~105.92 Mops, decode ~109.96 Mops.
- CSV: appended under `header_encode_Mops` and `header_decode_Mops`.

## Stage B — Updates (Block‑Level)

### RX Workspace Coalescing
- Consolidated `whitened[]`, `payload_crc[]`, and `tmp[]` into a single `bytes[]` buffer in `lora_rx_workspace`.
- In‑place dewhitening; CRC computed by zeroing trailing two bytes in‑place (restored after), avoiding extra copies.
- Files: `src/lora_chain.h`, `src/lora_rx_chain.c`.
- Chain tests pass: `test_lora_chain`, `test_end_to_end_file`, `test_sync_end_to_end`.

### Demod Copy Reduction
- Scan FFT spectrum directly from `fft.work` to avoid final memcpy.
- File: `src/lora_fft_demod.c`. `test_fft_demod_correctness` and chain tests pass.

### Hamming — Branchless Syndrome Fix
- Replaced switch with 8‑entry syndrome→fix table (CR≥3). File: `src/lora_hamming.c`. Tests pass.

### Whitening — 8‑Byte LUT (opt‑in)
- Added `lora_whiten_lut8`/`lora_whiten_next8`; guard `LORA_LITE_WHITEN_LUT8`.
- Near parity with SIMD on host (seq ~322.46 MB/s vs LUT8 ~318.80 MB/s); keep OFF on host.

### Gray Map/Demap — Micro‑bench
- Added `bench_graymap` (Msym/s): sf7 296.54 / 299.85; sf8 293.50 / 306.03; sf9 307.18 / 302.67; sf10 301.26 / 304.33; sf11 304.87 / 297.83; sf12 296.46 / 295.42.
- CSV updated under tag `fft_target,graymap_bench`.

### New Micro‑benches — Payload ID, CRC, Whitening (Embedded Profile)
- Added append patterns to Payload ID bench to quantify formatting and string composition overheads.
- Added embedded‑profile variants for CRC and Whitening, plus a dedicated whitening variants bench that contrasts sequence XOR vs LUT8 (dynamic vs static LUT).

Results (this host; embedded profile `-Os -flto` for CRC/whitening variants):

- Payload ID (O3):
  - u8_to_dec: 1433.6 Mops; snprintf: 44.0 Mops (≈32.6× faster)
  - Append‑after ("MSG:"+id): u8_to_dec 809.6 vs snprintf 43.2 Mops (≈18.8×)
  - Append‑before (id+":HELLO"): u8_to_dec 877.2 vs snprintf 42.9 Mops (≈20.4×)
  - Takeaway: prefer branchless `u8_to_dec` for 0..255; composition cost amortizes but does not erase the win.

- CRC (embedded profile):
  - Throughput ≈70.7 MB/s on this host build.
  - Takeaway: CRC is not a hotspot relative to whitening/FFT; no action needed now.

- Whitening variants (embedded profile):
  - Sequence XOR: 2564.7 MB/s
  - LUT8 dynamic (runtime‑built tables): 5682.0 MB/s (≈2.2× vs seq)
  - LUT8 static (flash‑resident tables): 8429.3 MB/s (≈1.48× vs dyn; ≈3.3× vs seq)
  - Takeaway: On this host, static LUT8 yields the best throughput and removes init time; enable `LORA_LITE_WHITEN_LUT8=ON` and `LORA_LITE_WHITEN_LUT8_STATIC=ON` in embedded presets, keep OFF on generic host builds.

Artifacts:
- `tests/benchmarks/bench_payload_id.c` extended with append‑before/after cases.
- `tests/benchmarks/bench_crc.c` reused; added `bench_crc_emb` target with embedded flags.
- New `tests/benchmarks/bench_whitening_emb_variants.c` to measure seq vs LUT8 dyn vs LUT8 static without toggling library flags.
- Appended CSV to `bench_emb.csv` with the metrics above.

### Interleaver/Mod — Embedded Benches
- Added embedded‑profile targets for interleaver and mod benches (`bench_interleaver_emb`, `bench_mod_emb`).
- Results on this host (embedded flags, Release):
  - Interleave (sf7,cw8): blocks/s ≈ 2.20e6
  - Mod (sf7, os=8): µs per symbol ≈ 28.67
- CSV appended: `interleave_blocks_per_sec_sf7_cw8`, `mod_us_per_sym_sf7_os8`.

### Frame Sync — Sliding Window Preamble
- Optimized `lora_frame_sync_find_preamble()` using a sliding-window counter to avoid branchy backtracking; deterministic O(n) passes.
- Micro-bench `bench_frame_sync` (host O3): `frame_sync_find_preamble_Mops ≈ 3.58` on 4K-symbol buffers with 64-symbol preamble.
- Tests: `test_lora_frame_sync{,_nolog}` and full chain remain green.
- Takeaway: fewer branches, simpler control flow, preserves return semantics (index after preamble-like region).

#### Align API — Offset Variant (no copy)
- Added `lora_frame_sync_align_offset()` which returns the start offset and length after the preamble without copying symbols.
- Integrated into `lora_rx_chain` to avoid a duplicate preamble scan and reduce transient work when determining `sym_off`.

### Full-Frame Symbols — Micro‑bench
- Added `bench_full_frame` to measure a full symbol-domain pipeline: preamble+SFD sync → bytes view → dewhiten → CRC validate.
- Result (host O3, preamble=8, payload=128): `full_frame_pipeline_Mfps ≈ 4.159`, all frames validated.
- CSV appended: `full_frame_pipeline_Mfps`, `full_frame_ok`.

### SFD — Adaptive Lookahead (bench)
- Updated `lora_frame_sync_find_sfd()` to slide an SFD-length window within a bounded lookahead and accept the first window meeting the non-zero threshold. This improves robustness to occasional zero-like glitches inside the SFD span without adding copies or rescans.
- Micro-bench `bench_sfd`:
  - `sfd_Mops_look2 ≈ 104.44`, `sfd_Mops_look8 ≈ 100.28` (host O3; synthesized sequences with 1 injected zero in SFD)
  - Index sums identical across lookaheads, confirming consistent detection.
- Takeaway: Adaptive lookahead maintains high throughput with minor overhead while handling small SFD perturbations.

### Frame Sync Quality — Metrics (bench)
- Added `lora_frame_sync_analyze()` that reports preamble match % and SFD nonzero count, and `bench_frame_sync_quality` to exercise it under noise.
- Results (host O3; 20k trials, preamble=16, SFD=2):
  - 0% noise: `fsq_Mops ≈ 1.53`, avg preamble match 100%, SFD nonzero ≈ 2.0
  - 5% noise: `fsq_Mops ≈ 1.57`, avg preamble match 100%, SFD nonzero ≈ 2.0
  - 15% noise: `fsq_Mops ≈ 1.48`, avg preamble match 100%, SFD nonzero ≈ 2.0
- Takeaway: with current symbol-domain tolerance, preamble and SFD metrics remain stable under light/moderate noise in this model; on real targets, tie thresholds (e.g., `LORA_FS_MIN_MATCH_PCT`, SFD nonzero) per SF/BW/profile as needed.

### TX Workspace — No-Malloc Path + Sizing Helper
- Added `lora_tx_workspace_bytes(const lora_chain_cfg*)` returning the TX workspace size (currently configuration‑independent).
- Enforced no dynamic allocations in `lora_tx_run()` when `LORA_LITE_NO_MALLOC` is defined: uses static buffers for input payload and output chips, and a static `lora_tx_workspace`.
- Preserved host fallback: when `LORA_LITE_NO_MALLOC` is not defined, `lora_tx_run()` retains `malloc`/`free` to avoid large static buffers in generic builds.

Results
- TX runtime allocations: 0 (embedded build with `LORA_LITE_NO_MALLOC`).
- Workspace bytes: `sizeof(lora_tx_workspace) = (LORA_MAX_PAYLOAD_LEN+2) + 4*LORA_MAX_NSYM = 256 + 1024 = 1280` bytes.
- Chips buffer size (unchanged, caller‑owned in run helper): `LORA_MAX_CHIPS*sizeof(cf)`.

Takeaways
- TX path now mirrors RX guardrails: no hidden allocations under embedded profile, predictable RAM use, and backwards‑compatible host behavior.

## No‑Malloc Profile — Test Matrix
- Built with `-DLORA_LITE_NO_MALLOC=ON` and ran `ctest -V`.
- Initial run: 84% tests passed (53/63), 10 failed (chain/integration level) due to missing RX workspace.
- After adapting tests to pass `fft_ws`:
  - Passed: `test_lora_chain{,_nolog}`, `test_end_to_end_file{,_nolog}`, `test_sync_end_to_end{,_nolog}`, `test_ber_snr{,_nolog}`.
  - Still failing: `embedded_loopback{,_nolog}`.
- Debug (embedded_loopback): printed `need=10240`, `ws_size=10240..16384`, `ws!=NULL`; yet `lora_rx_chain` returns `LORA_ERR_BUFFER_TOO_SMALL`. Other chain tests with same cfg pass, suggesting a test‑specific nuance; will deep‑dive separately (suspect local alignment/size check drift or test ordering/state).

Final run (after loopback fix):
- Removed mismatched `LORA_LITE_FIXED_POINT` definition from the loopback test to avoid ABI drift in `lora_rx_workspace` layout.
- Provided static aligned workspace in the test (`_Alignas(32) unsigned char rx_fft_ws_buf[16384]`).
- Full `ctest` with `-DLORA_LITE_NO_MALLOC=ON`: 63/63 passed.
- CSV appended: `ctest_no_malloc_passed=63`, `ctest_no_malloc_failed=0`.

Conclusions
- Enforcement works: RX/TX do not fall back to dynamic allocations under `LORA_LITE_NO_MALLOC`; instead they fail fast when workspace is absent, confirming no hidden `malloc` paths in the chain helpers.
- Next step: finalize `embedded_loopback` for no‑malloc (either static aligned WS or minor test refactor). All other chain/integration tests pass with provided workspaces.

## Tooling & Presets
- CMake presets: `arm-embedded-release` (env `ARM_TOOLCHAIN_FILE`), `aarch64-embedded-release` (env `AARCH64_TOOLCHAIN_FILE`).
- Scripts: `scripts/run_fft_bench_remote.sh`, `scripts/run_chain_bench_remote.sh`, `scripts/run_chain_bench_local.sh` to build/copy/run benches and append results to `bench_emb.csv`.

## ARM/NEON Hooks
- Guarded NEON complex mul in hot radix‑2 butterflies and prefetch hints. Guards: `LORA_LITE_NEON`, `LORA_LITE_EMB_THROUGHPUT`.

## RO Tables Consolidation
- Current: Gray LUT (`src/lora_gray_lut.h`) and CRC table (`src/lora_crc_table.h`) in `.rodata`; whitening LUT8 built at init.
- Plan: consolidate per‑SF/CR read‑only tables into a single translation unit for locality; gate optional large LUTs with flags to balance flash vs cycles.

## Next — FFT Block
- Add `restrict` and alignment hints to FFT paths. DONE
- Provide bit-reversal mode knob: copy-once in bit-reversed order (default) vs in-place bit-reversal of a straight copy (`LORA_LITE_FFT_BITREV_COPY=OFF`).
- Keep liquid/FFTW backend unchanged; add CMSIS-DSP hooks later behind `LORA_LITE_USE_CMSIS`.

### FFT changes implemented
- New compile flag `LORA_LITE_FFT_BITREV_COPY` (default ON) to select between copy-into-bitrev or copy+inplace-bitrev.
- Skip final memcpy when `out` aliases the internal `work` buffer.
- Local `restrict` views and `__builtin_assume_aligned(32)` hints for `work`, `tw`, `in`, `out` in `lora_fft_exec_fwd`.
- Faster bit-reversal index: when `LORA_LITE_FFT_BITREV_COPY` is ON, bit-reversed indices are computed with a table-accelerated 8-bit reversal (or compiler builtin) instead of a per-bit loop. Reduces front-end overhead for SF7–SF12.
- Power-of-two guard: `lora_fft_init()` now rejects non power-of-two sizes.
- Workspace sizing helper: `lora_fft_core_workspace_bytes(n)` added to the public header to simplify buffer provisioning in embedded builds (distinct from demod’s `lora_fft_workspace_bytes(sf,fs,bw)`).
- Inverse FFT: added `lora_fft_exec_inv()` with a Liquid backward plan when enabled; falls back to a portable inverse using conjugate twiddles and applies 1/N scaling.
- Unit test: `tests/test_fft_roundtrip.c` validates fwd→inv round-trip across SF7–SF12 with tight error bounds.
- Optional Radix-4: added radix-4 passes guarded by `LORA_LITE_FFT_RADIX4` (enable with `-DLORA_LITE_FFT_RADIX4=ON`). Falls back to radix-2 when `n` not divisible by 4.

### CMSIS-DSP Q15 backend hook (scaffold)
- Added `include/lora_fft_q15.h` and `src/lora_fft_q15.c`.
- CMake checks for `arm_math.h`; defines `LORA_LITE_HAVE_CMSIS_HEADERS` when present.
- If built with `-DLORA_LITE_USE_CMSIS=ON` and CMSIS headers are available, `lora_fft_q15_exec_fwd()` uses CMSIS Q15 CFFT; otherwise it falls back to float FFT conversion internally.
- Not yet wired into `lora_fft_demod` (next step will gate a Q15 FFT path in the fixed-point demod and compare performance/accuracy).

### Demod integration of Q15 FFT
- `lora_fft_demod` now optionally uses the Q15 FFT when both `LORA_LITE_FIXED_POINT` and `LORA_LITE_USE_CMSIS` are enabled.
- Path details:
  - Accumulates per-bin dechirped samples into `bins_q15`.
  - If CMSIS is available, executes Q15 FFT in-place and converts the spectrum to float once for magnitude search.
  - Otherwise, falls back to the existing path (convert to float, run float FFT).
- Tests updated to link the Q15 FFT object where needed; all fixed/float equivalence and correctness tests pass.

### Bench: Bit-reversal mode comparison (kiss backend)
- Command:
  - ON (copy into bitrev): `cmake -B build-fft-on -DLORA_LITE_BENCHMARK=ON -DLORA_LITE_USE_LIQUID_FFT=OFF -DLORA_LITE_FFT_BITREV_COPY=ON && ./build-fft-on/tests/bench_fft > bench_out/bench_fft_ON.csv`
  - OFF (copy + in-place bitrev): `cmake -B build-fft-off -DLORA_LITE_BENCHMARK=ON -DLORA_LITE_USE_LIQUID_FFT=OFF -DLORA_LITE_FFT_BITREV_COPY=OFF && ./build-fft-off/tests/bench_fft > bench_out/bench_fft_OFF.csv`
- Results (us per exec):
  - `bench_out/bench_fft_ON.csv`
  - `bench_out/bench_fft_OFF.csv`
- Observation: ON is modestly faster across SF7–SF12 on host; keep ON as default.

### Next steps (FFT)
- Radix-4 butterflies when `n % 4 == 0` (fall back to radix-2 otherwise).
- Stage-local twiddle stride precomputation to trim index math in inner loops.
- Optional NEON path for complex FMA on ARM (guarded by `LORA_LITE_EMB_THROUGHPUT`).
- Add iFFT path for Q15 (CMSIS and portable fallback) to enable fixed-point round-trip testing.

### Bench: Q15 FFT vs float (host fallback and ARM/QEMU)
- Host (fallback float implementation):
  - Build: `cmake -B build -DLORA_LITE_BENCHMARK=ON -DLORA_LITE_FIXED_POINT=ON`
  - Run: `LD_LIBRARY_PATH=build/src ./build/tests/bench_fft_q15 > bench_out/bench_fft_q15_host.csv`
  - Files: `bench_out/bench_fft_q15_host.csv`
  - Note: On host without CMSIS, Q15 path converts to float internally; timing reflects overhead, and error metrics include Q15 quantization/scale.
- ARM/QEMU (when CMSIS available in sysroot):
  - Script: `scripts/run_qemu_aarch64.sh` builds for aarch64 and runs `bench_lora_chain`, `bench_fft`, and (if present) `bench_fft_q15` under QEMU, producing:
    - `results/arm_qemu_chain.csv`, `results/arm_qemu_fft.csv`, `results/arm_qemu_fft_q15.csv`.
  - To enable CMSIS path, configure with `-DLORA_LITE_USE_CMSIS=ON` and provide `arm_math.h` in the cross toolchain sysroot.

Preset Update (CMSIS ON by default for embedded presets)
- Updated `CMakePresets.json` so `arm-embedded-release` ו־`aarch64-embedded-release` מגדירים `LORA_LITE_USE_CMSIS=ON`. כאשר `arm_math.h` קיים בסביבת הקרוס, בנץ’ `bench_fft_q15` ותוואי ה־Q15 ישתמשו ב־CMSIS; אחרת יפעל fallback.

Remote ARM run helper:
- Script `scripts/run_fft_q15_remote.sh PRESET user@host:/remote/dir [tag]` builds with the given preset, copies `bench_fft_q15` + `liblora_fft.so` to the target, runs it with `LD_LIBRARY_PATH=.`, fetches the output, and appends to `bench_emb.csv` with the given tag.
- Example:
  - `ARM_TOOLCHAIN_FILE=</path/to/arm/toolchain.cmake> ./scripts/run_fft_q15_remote.sh arm-embedded-release user@armdev:/tmp/lora_bench cmsis_arm_som`
- CSV rows appended:
  - `fft_q15_target,<tag>`
  - `q15_backend,cmsis_or_fallback`
  - `q15_us_per_exec_sfX,...`, `float_us_per_exec_sfX,...`
  - `q15_vs_float_max_abs_sfX,...`, `q15_vs_float_rms_sfX,...`

## Interleaver — readiness check
- Current implementation already uses precomputed permutation tables and processes in-place per block without allocations.
- Minor tweak: added `restrict` to `lora_deinterleave()` prototype and implementation for better alias analysis (no behavior change).
- Existing micro-benchmark: `bench_interleaver` prints blocks/sec; correctness is verified by round-trip interleave→deinterleave.
### Signal/IO/Utils — Q15 Scaling Unification
- Unified Q15 rounding/clamping across modules to avoid magic numbers and divergences:
  - Added constants/macros in `src/lora_fixed.h`: `LORA_Q15_SCALE_F`, `LORA_Q15_CLAMP_MAX`, etc.
  - Refactored Q15 conversions to use `liquid_float_to_fixed()` / `liquid_fixed_to_float()` instead of ad‑hoc code in:
    - `src/lora_mod.c` (fixed path float↔Q15 emulation)
    - `src/lora_fft_demod.c` (downchirp quantization)
    - `src/lora_rx_chain.c` (scalar and NEON tail conversion paths)
- Tests (no‑malloc build and focused subset) passed: `test_fft_demod_correctness`, chain tests, and loopback.
- Takeaway: consistent Q15 handling reduces drift across modules and simplifies future SIMD/back‑end swaps without behavior skew.

### RX Pipeline Copies — Micro‑bench
- Added `bench_rx_bytes_pipeline` to quantify the RX bytes path (dewhiten + CRC check), comparing the optimized in-place flow vs a copy-heavy baseline.
- Host (O3) and embedded-profile (Os+LTO) results on this machine:
  - In-place: ~59–60 MB/s
  - Copy-heavy: ~60 MB/s (similar on this host due to memory bandwidth/cache; on embedded targets expect in-place to win by avoiding extra writes).
- CSV appended under: `rx_pipeline_inplace_MBps`, `rx_pipeline_copy_MBps` (both for O3 and embedded variants).
- Takeaway: On this host, memcpy cost masks small gains; on MCUs/SOCs (slower memory), in-place should reduce cycles and power for the same throughput. The chain already uses in-place; we keep it.

### TX Pipeline Copies — Micro‑bench
- Added `bench_tx_bytes_pipeline` to measure TX flow (CRC add + whiten) in-place vs copy-heavy baseline.
- Host O3 vs Embedded flags (this machine):
  - O3: in-place ≈ 58.8 MB/s, copy ≈ 59.2 MB/s
  - Embedded: in-place ≈ 58.0 MB/s, copy ≈ 58.6 MB/s
- CSV appended under: `tx_pipeline_inplace_MBps`, `tx_pipeline_copy_MBps`.
- Takeaway: כמו ב-RX, על ה-host ההפרש קטן (Memory-bound). על יעדים מש嵌דים, in-place צפוי להיטיב. מסלול ה-TX כבר in-place, כך שאיננו מוסיפים העתקות.
