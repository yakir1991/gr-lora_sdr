#!/usr/bin/env python3  # Ensure the script executes with the Python 3 interpreter.
# """Generate the reference chirps used by the LoRa transmitter chain."""  # Summarise the purpose of the module in English.
#
# The C++ modulator block in :mod:`gnuradio.lora_sdr` generates reference
# upchirp and downchirp waveforms through ``build_ref_chirps``. This script
# reproduces the same computation in Python so that a user can export the
# reference vectors for further processing, testing or comparison.
#
# The script stores the upchirp (and optionally the downchirp) to disk and can
# print the first few samples for quick inspection.
"""Generate the reference chirps used by the LoRa transmitter chain."""

from __future__ import annotations  # Delay evaluation of annotations for forward references.

import argparse  # Parse command line options controlling the chirp generation process.
import math  # Provide mathematical helpers for oversampling validation.
from pathlib import Path  # Manipulate filesystem paths in a platform-independent manner.
from typing import Iterable, Tuple  # Provide type annotations for iterables returned by helper functions.

import numpy as np  # Use NumPy to compute complex chirp vectors efficiently.


def _infer_os_factor(samp_rate: float, bandwidth: float) -> int:
    """Derive the integer oversampling factor used by the modulator."""

    if bandwidth <= 0:
        raise ValueError("Bandwidth must be strictly positive")  # Validate the provided bandwidth parameter.

    ratio = samp_rate / bandwidth  # Compute how many times faster the sampling runs compared to the bandwidth.
    rounded = int(round(ratio))  # Round the ratio to the nearest integer candidate.
    if not math.isclose(ratio, rounded, rel_tol=0, abs_tol=1e-9):
        raise ValueError(
            "The sampling rate must be an integer multiple of the bandwidth "
            f"(got ratio={ratio!r})"
        )  # Enforce the oversampling requirement strictly.
    if rounded <= 0:
        raise ValueError("The derived oversampling factor must be positive")  # Reject non-positive oversampling factors.
    return rounded  # Provide the valid oversampling factor back to the caller.


def build_reference_chirps(sf: int, os_factor: int) -> Tuple[np.ndarray, np.ndarray]:
    """Return the reference upchirp and downchirp used by the TX chain."""

    if sf <= 0:
        raise ValueError("The spreading factor must be positive")  # Guard against invalid spreading factor values.
    if os_factor <= 0:
        raise ValueError("The oversampling factor must be positive")  # Ensure oversampling is defined.

    samples_per_symbol = (1 << sf) * os_factor  # Compute the number of time samples in a single LoRa symbol.
    n = np.arange(samples_per_symbol, dtype=np.float64)  # Build an array of sample indices with floating precision.
    n_os = float(os_factor)  # Convert the oversampling factor to float for the phase expression.
    symbol_bins = float(1 << sf)  # Determine the number of discrete frequency bins used in LoRa modulation.

    phase = 2.0 * np.pi * ((n * n) / (2.0 * symbol_bins * n_os * n_os) - 0.5 * n / n_os)  # Calculate the chirp phase progression.
    upchirp = np.exp(1j * phase).astype(np.complex64)  # Construct the complex upchirp waveform using Euler's formula.
    downchirp = np.conj(upchirp)  # Obtain the downchirp by conjugating the upchirp.
    return upchirp, downchirp  # Provide both chirp vectors for further processing.


def save_chirps(
    upchirp: np.ndarray,
    downchirp: np.ndarray,
    output: Path,
    fmt: str,
    include_down: bool,
) -> Iterable[Path]:
    """Persist the computed chirps using the requested format."""

    output = output.expanduser().resolve()  # Normalize the destination path to an absolute path.
    produced = []  # Collect paths to every file written during the export process.

    if fmt == "npz":
        target = output  # Start with the user-provided output path.
        if target.suffix != ".npz":
            target = target.with_suffix(".npz")  # Guarantee the archive uses the expected extension.
        if include_down:
            np.savez(target, upchirp=upchirp, downchirp=downchirp)  # Store both chirps in a single compressed archive.
        else:
            np.savez(target, upchirp=upchirp)  # Save only the upchirp when the downchirp is disabled.
        produced.append(target)  # Record the archive path for reporting.
    elif fmt == "npy":
        base = output  # Use the output path as a base name for .npy files.
        if base.suffix:
            base = base.with_suffix("")  # Remove any suffix to avoid duplication.
        up_path = base.with_name(base.name + "_up.npy")  # Compose the file path for the upchirp array.
        np.save(up_path, upchirp)  # Save the upchirp samples to disk.
        produced.append(up_path)  # Track the generated file.
        if include_down:
            down_path = base.with_name(base.name + "_down.npy")  # Compose the file path for the downchirp array.
            np.save(down_path, downchirp)  # Save the downchirp samples to disk.
            produced.append(down_path)  # Track the additional file.
    elif fmt == "txt":
        base = output  # Use the output path as a base name for text files.
        if base.suffix:
            base = base.with_suffix("")  # Strip user-provided suffixes before appending our own.
        up_path = base.with_name(base.name + "_up.txt")  # Compose the filename for the upchirp text export.
        np.savetxt(
            up_path,
            np.column_stack((upchirp.real, upchirp.imag)),
            fmt="%.12e",
            header="real\timag",
        )  # Write the real and imaginary parts as columns in a text file.
        produced.append(up_path)  # Record the exported text file.
        if include_down:
            down_path = base.with_name(base.name + "_down.txt")  # Compose the filename for the downchirp text export.
            np.savetxt(
                down_path,
                np.column_stack((downchirp.real, downchirp.imag)),
                fmt="%.12e",
                header="real\timag",
            )  # Save the downchirp samples in text form as well.
            produced.append(down_path)  # Track the created file path.
    else:
        raise ValueError(f"Unsupported export format: {fmt}")  # Report unknown export formats as errors.

    return produced  # Provide the list of produced files to the caller.


def format_complex(value: complex) -> str:
    return f"{value.real:+.6f} {value.imag:+.6f}j"  # Present a complex number with signed fixed-precision components.


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Export the reference LoRa chirps generated by the transmitter chain.",
    )  # Create the command-line parser used to configure the script.
    parser.add_argument("--sf", type=int, default=7, help="Spreading factor (default: 7)")  # Allow users to select the LoRa spreading factor.
    parser.add_argument(
        "--bw",
        type=float,
        default=125000,
        help="Transmission bandwidth in Hz (used when deriving the oversampling factor)",
    )  # Accept the LoRa channel bandwidth for oversampling calculations.
    parser.add_argument(
        "--samp-rate",
        type=float,
        default=None,
        help="Sampling rate in Hz. Required when --os-factor is not provided.",
    )  # Enable specifying the sample rate when inferring oversampling.
    parser.add_argument(
        "--os-factor",
        type=int,
        default=None,
        help="Explicit oversampling factor (samples per symbol bin). Overrides --samp-rate/--bw.",
    )  # Permit overriding the inferred oversampling factor.
    parser.add_argument(
        "--output",
        type=Path,
        default=None,
        help="Destination file or prefix. Defaults to reference_chirps_sf{sf}_os{os}.npz",
    )  # Control where the generated files are saved.
    parser.add_argument(
        "--format",
        choices=("npz", "npy", "txt"),
        default="npz",
        help="File format used to save the chirps (default: npz)",
    )  # Choose the serialization format for the chirp data.
    parser.add_argument(
        "--no-downchirp",
        action="store_true",
        help="Only export the reference upchirp",
    )  # Allow skipping downchirp generation and export.
    parser.add_argument(
        "--preview",
        action="store_true",
        help="Print the first few samples of each chirp to stdout",
    )  # Request a textual preview of the generated samples.

    args = parser.parse_args()  # Parse the provided command-line options into the args namespace.

    if args.os_factor is not None:
        os_factor = args.os_factor  # Use the explicitly provided oversampling factor.
    else:
        if args.samp_rate is None:
            raise ValueError("Either --os-factor or --samp-rate must be specified")  # Ensure enough information to compute oversampling.
        os_factor = _infer_os_factor(args.samp_rate, args.bw)  # Derive the oversampling factor from the sampling rate and bandwidth.

    upchirp, downchirp = build_reference_chirps(args.sf, os_factor)  # Generate the transmitter reference chirps.

    if args.output is None:
        default_name = f"reference_chirps_sf{args.sf}_os{os_factor}"  # Construct a default filename stem when none is given.
        suffix = ".npz" if args.format == "npz" else ""  # Append the default extension for NPZ outputs.
        output = Path(default_name + suffix)  # Combine the stem and suffix into a path.
    else:
        output = args.output  # Use the user-specified output path as-is.

    saved_files = save_chirps(upchirp, downchirp, output, args.format, not args.no_downchirp)  # Persist the chirps according to the format options.

    print(
        f"Generated reference chirps with {len(upchirp)} samples per symbol "
        f"(SF={args.sf}, oversampling={os_factor})."
    )  # Summarise the chirp properties for the user.
    for path in saved_files:
        print(f"Saved {path}")  # Report each file written to disk.

    if args.preview:
        preview_len = min(8, len(upchirp))  # Determine how many samples to display without exceeding the chirp length.
        print("First samples of the upchirp:")  # Introduce the upchirp preview section.
        for idx in range(preview_len):
            print(f"  up[{idx}] = {format_complex(upchirp[idx])}")  # Print each selected upchirp sample in human-readable form.
        if not args.no_downchirp:
            print("First samples of the downchirp:")  # Introduce the downchirp preview when it was generated.
            for idx in range(preview_len):
                print(f"  down[{idx}] = {format_complex(downchirp[idx])}")  # Display the corresponding downchirp samples.


if __name__ == "__main__":
    main()  # Run the command-line interface when the script is executed directly.
