#!/usr/bin/env python3  # Use the Python 3 interpreter when running the script directly.
# """Decode an offline LoRa baseband capture using the receiver chain."""  # Module docstring announcing the tool's purpose.
#
# The script reads complex32 IQ samples from disk, feeds them to the
#  :class:`gnuradio.lora_sdr.lora_sdr_lora_rx` hierarchical block and prints
#  the recovered payloads together with CRC information.
"""Decode an offline LoRa baseband capture using the receiver chain."""

from __future__ import annotations  # Enable postponed evaluation of annotations for forward references.

import argparse  # Parse command line arguments describing the capture and decoding options.
import re  # Provide regular-expression parsing for filename-based parameter inference.
from dataclasses import dataclass  # Supply the decorator used to store frame metadata.
from pathlib import Path  # Manipulate filesystem paths in an OS-agnostic manner.
from typing import Dict, Iterable, List, Optional, Sequence, Tuple  # Type hints for clarity in the script.

import numpy as np  # Load IQ samples efficiently from disk using NumPy arrays.
import pmt  # Handle GNU Radio PMT message types for tag decoding and message handling.
from gnuradio import blocks, gr  # Import the base GNU Radio runtime classes and standard processing blocks.
import gnuradio.lora_sdr as lora_sdr  # Bring in the custom LoRa SDR module providing transmitter/receiver blocks.


@dataclass  # Automatically generate an initializer and representation for the container class.
class FrameResult:
    index: int  # Sequential frame index extracted from the receive stream.
    payload: bytes  # Raw payload bytes recovered from the LoRa frame.
    crc_valid: Optional[bool]  # Whether the CRC check succeeded (True), failed (False), or is unavailable (None).
    has_crc: Optional[bool]  # Flag showing whether the frame actually carried a CRC field.
    message_text: Optional[str]  # Optional decoded textual representation of the payload for display.

    def hex_payload(self) -> str:
        return " ".join(f"{byte:02x}" for byte in self.payload)  # Render the payload as a space-separated hexadecimal string.


class MessageCollector(gr.sync_block):  # Define a synchronous block used solely for message collection.
    """Collect messages emitted on a GNU Radio message port."""

    def __init__(self):
        super().__init__(name="message_collector", in_sig=None, out_sig=None)  # Initialize the block without stream inputs/outputs.
        self.message_port_register_in(pmt.intern("in"))  # Create an input message port named "in".
        self.set_msg_handler(pmt.intern("in"), self._handle_msg)  # Attach the handler that stores arriving messages.
        self.messages: List[bytes] = []  # Hold the raw byte representation of all received messages.

    def _handle_msg(self, msg: pmt.pmt_t) -> None:
        if pmt.is_symbol(msg):  # Handle PMT symbols that contain printable text.
            text = pmt.symbol_to_string(msg)  # Convert the symbol to a Python string.
            self.messages.append(text.encode("latin-1", errors="ignore"))  # Store the text encoded as bytes, ignoring invalid characters.
        elif pmt.is_u8vector(msg):  # Detect byte-vector payloads that already represent raw data.
            self.messages.append(bytes(pmt.u8vector_elements(msg)))  # Convert the PMT vector into a bytes object.
        else:  # For all other message types fall back to serialising the PMT object.
            self.messages.append(pmt.serialize_str(msg))  # Serialize the PMT into bytes to avoid losing information.

    def work(self, input_items, output_items):  # pragma: no cover - message only block
        return 0  # The block does not process streaming data, so it reports zero output items.


class OfflineLoraReceiver(gr.top_block):  # Assemble a flowgraph that decodes the stored IQ samples offline.
    def __init__(
        self,
        iq_samples: Sequence[complex],  # Vector of IQ samples captured from disk.
        samp_rate: float,  # Sample rate of the capture in samples per second.
        bw: float,  # Signal bandwidth used during the capture.
        sf: int,  # LoRa spreading factor to decode.
        cr: int,  # Coding rate configured during capture.
        has_crc: bool,  # Flag specifying whether frames include a CRC.
        impl_head: bool,  # Flag selecting implicit vs explicit headers.
        ldro_mode: int,  # Low data-rate optimisation configuration.
        sync_word: Sequence[int],  # Receiver sync-word pattern.
        pay_len: int,  # Maximum payload length to expect.
        soft_decoding: bool = False,  # Optionally enable soft decision decoding.
        center_freq: float = 868.1e6,  # Default centre frequency used for metadata and prints.
        print_header: bool = True,  # Whether the RX block should print header information.
        print_payload: bool = True,  # Whether the RX block should print payload contents.
    ) -> None:
        super().__init__("offline_lora_receiver", catch_exceptions=True)  # Initialise the top block with exception handling enabled.

        self.src = blocks.vector_source_c(list(iq_samples), False)  # Feed the stored complex samples into the flowgraph as a finite stream.
        self.rx = lora_sdr.lora_sdr_lora_rx(
            center_freq=center_freq,  # Configure the receiver centre frequency for logging.
            bw=int(bw),  # Cast bandwidth to integer as expected by the block API.
            cr=cr,  # Set the coding rate parameter.
            has_crc=has_crc,  # Inform the receiver whether frames carry a CRC.
            impl_head=impl_head,  # Pass the implicit-header flag through.
            pay_len=pay_len,  # Tell the block the maximum payload length to allocate buffers for.
            samp_rate=int(samp_rate),  # Provide the sampling rate, coercing to integer samples per second.
            sf=sf,  # Supply the spreading factor to use during demodulation.
            sync_word=list(sync_word),  # Convert the sync-word sequence into a mutable list for the block.
            soft_decoding=soft_decoding,  # Select whether to output soft bits.
            ldro_mode=ldro_mode,  # Configure the low data-rate optimisation behaviour.
            print_rx=[print_header, print_payload],  # Control internal printing of header and payload information.
        )
        self.payload_sink = blocks.vector_sink_b()  # Collect the decoded payload bytes emitted by the receiver block.
        self.msg_collector = MessageCollector()  # Capture message-port outputs generated by the receiver.

        self.connect(self.src, self.rx)  # Route the sample stream into the receiver hierarchy block.
        self.connect(self.rx, self.payload_sink)  # Store the byte stream produced by the receiver.
        self.msg_connect((self.rx, "out"), (self.msg_collector, "in"))  # Attach the receiver message port to the collector.

    def results(self) -> Tuple[bytes, Iterable[gr.tag_t], List[bytes]]:
        payload_bytes = bytes(self.payload_sink.data())  # Gather all payload bytes received during the run.
        return payload_bytes, list(self.payload_sink.tags()), list(self.msg_collector.messages)  # Return payloads, associated tags, and messages.


def parse_numeric_suffix(value: str) -> float:
    match = re.fullmatch(r"(?P<base>\d+(?:\.\d+)?)(?P<suffix>[kKmMgG]?)", value)  # Match numbers optionally followed by magnitude suffixes.
    if not match:
        raise ValueError(f"Cannot parse numeric value '{value}'")  # Report invalid strings that do not comply with the expected pattern.
    base = float(match.group("base"))  # Convert the numeric portion to a floating-point value.
    suffix = match.group("suffix").lower()  # Normalise the suffix letter to lowercase for lookup.
    scale = {"": 1.0, "k": 1e3, "m": 1e6, "g": 1e9}[suffix]  # Map suffix letters to their decimal multiplier.
    return base * scale  # Combine the magnitude with the base number.


def infer_parameters_from_name(stem: str) -> Dict[str, object]:
    pattern = re.compile(
        r"sps_(?P<sps>[^_]+)_bw_(?P<bw>[^_]+)_sf_(?P<sf>\d+)_cr_(?P<cr>\d+)"
        r"_ldro_(?P<ldro>[^_]+)_crc_(?P<crc>[^_]+)_implheader_(?P<impl>[^_]+)"
    )  # Build a pattern that recognises capture filenames storing parameters.
    match = pattern.search(stem)  # Attempt to extract parameter groups from the filename stem.
    if not match:
        return {}  # Without a match we cannot infer any parameters.

    params: Dict[str, object] = {
        "samp_rate": parse_numeric_suffix(match.group("sps")),  # Decode and convert the samples-per-symbol field.
        "bw": parse_numeric_suffix(match.group("bw")),  # Extract and scale the bandwidth indicator.
        "sf": int(match.group("sf")),  # Read the spreading factor as an integer.
        "cr": int(match.group("cr")),  # Recover the coding rate from the filename.
        "ldro_mode": 1 if match.group("ldro").lower() == "true" else 0,  # Translate LDRO flag into the expected mode.
        "has_crc": match.group("crc").lower() == "true",  # Determine whether the capture included a CRC.
        "impl_head": match.group("impl").lower() == "true",  # Discover whether the header was implicit.
    }
    return params  # Provide the inferred parameters to the caller.


def extract_frames(payload_bytes: bytes, tags: Iterable[gr.tag_t]) -> List[FrameResult]:
    frames: List[FrameResult] = []  # Collect the parsed frames with metadata.
    cursor = 0  # Track the current position within the concatenated payload bytes.
    sorted_tags = sorted(tags, key=lambda t: t.offset)  # Process tags in ascending order of sample offset.

    for tag in sorted_tags:
        key = pmt.symbol_to_string(tag.key)  # Convert the tag key from PMT to string to identify its type.
        if key != "frame_info":
            continue  # Ignore tags that are unrelated to frame information.
        info = tag.value  # Retrieve the PMT dictionary containing metadata about the frame.
        pay_len_pmt = pmt.dict_ref(info, pmt.string_to_symbol("pay_len"), pmt.PMT_NIL)  # Look up the payload length entry.
        if pmt.is_integer(pay_len_pmt):
            pay_len = int(pmt.to_long(pay_len_pmt))  # Convert the payload length into a Python integer when present.
        else:
            pay_len = 0  # Default to zero length if the tag lacks the field.

        crc_present_pmt = pmt.dict_ref(info, pmt.string_to_symbol("crc"), pmt.PMT_NIL)  # Retrieve the CRC-present indicator.
        crc_present = pmt.to_bool(crc_present_pmt) if not pmt.is_null(crc_present_pmt) else None  # Interpret the CRC flag or fall back to None.

        crc_valid_pmt = pmt.dict_ref(info, pmt.string_to_symbol("crc_valid"), pmt.PMT_NIL)  # Fetch the CRC validation result.
        crc_valid = pmt.to_bool(crc_valid_pmt) if not pmt.is_null(crc_valid_pmt) else None  # Convert the validation outcome to Python bool/None.

        payload = payload_bytes[cursor:cursor + pay_len]  # Slice the payload bytes belonging to the current frame.
        cursor += pay_len  # Advance the cursor for the next frame extraction.

        frames.append(
            FrameResult(
                index=len(frames),  # Assign the frame index based on the number of frames already stored.
                payload=payload,  # Preserve the extracted payload bytes.
                crc_valid=crc_valid,  # Store whether the CRC passed.
                has_crc=crc_present,  # Remember if the CRC field was present at all.
                message_text=None,  # Initialise the textual representation to be filled later.
            )
        )

    return frames  # Provide the frame list to the caller.


def combine_messages(frames: List[FrameResult], messages: Sequence[bytes]) -> None:
    for frame, msg_bytes in zip(frames, messages):  # Pair each stored frame with the corresponding GNU Radio message.
        try:
            frame.message_text = msg_bytes.decode("utf-8")  # Prefer decoding payloads as UTF-8 strings.
        except UnicodeDecodeError:
            frame.message_text = msg_bytes.decode("latin-1", errors="replace")  # Fallback to Latin-1 while replacing invalid bytes.


def main() -> None:
    parser = argparse.ArgumentParser(description="Decode a stored LoRa IQ vector")  # Build a CLI argument parser describing the tool.
    parser.add_argument("input", type=Path, help="Path to the binary IQ capture")  # Require the path to the IQ file.
    parser.add_argument("--sf", type=int, help="Spreading factor")  # Allow overriding the spreading factor manually.
    parser.add_argument("--bw", type=float, help="Signal bandwidth in Hz")  # Accept a user-specified bandwidth.
    parser.add_argument("--samp-rate", type=float, help="Sampling rate in Hz")  # Allow the sampling rate to be provided explicitly.
    parser.add_argument("--cr", type=int, help="Coding rate (1-4)")  # Enable selection of the coding rate parameter.
    parser.add_argument("--has-crc", dest="has_crc", action="store_true", help="Force CRC usage")  # Provide a switch to require CRCs.
    parser.add_argument("--no-crc", dest="has_crc", action="store_false", help="Force CRC disabled")  # Provide a switch to disable CRC usage.
    parser.add_argument("--impl-header", dest="impl_head", action="store_true", help="Use implicit header")  # Toggle implicit header mode.
    parser.add_argument("--explicit-header", dest="impl_head", action="store_false", help="Use explicit header")  # Toggle explicit header mode.
    parser.add_argument("--ldro-mode", type=int, choices=[0, 1, 2], help="Low data-rate optimisation mode (0=off,1=on,2=auto)")  # Choose LDRO mode.
    parser.add_argument("--sync-word", type=lambda x: int(x, 0), default=0x12, help="Sync word as integer")  # Parse the sync word argument.
    parser.add_argument("--pay-len", type=int, default=255, help="Maximum payload length")  # Set the maximum expected payload size.
    parser.add_argument("--limit", type=int, default=None, help="Process only the first N IQ samples")  # Optionally limit the sample count.
    parser.set_defaults(has_crc=None, impl_head=None)  # Default optional boolean arguments to None to detect unspecified values.

    args = parser.parse_args()  # Parse all provided command-line arguments.

    file_path = args.input.expanduser().resolve()  # Expand user references and resolve the capture path to an absolute path.
    if not file_path.exists():
        raise FileNotFoundError(file_path)  # Abort if the capture file is missing.

    inferred = infer_parameters_from_name(file_path.stem)  # Attempt to deduce decoder parameters from the filename.

    sf = args.sf or inferred.get("sf")  # Choose the user-specified SF or fall back to inference.
    bw = args.bw or inferred.get("bw")  # Choose the provided bandwidth or inferred value.
    samp_rate = args.samp_rate or inferred.get("samp_rate", bw)  # Derive the sample rate from arguments, inference, or bandwidth default.
    cr = args.cr or inferred.get("cr", 1)  # Determine the coding rate with a default of 1.
    has_crc = args.has_crc if args.has_crc is not None else inferred.get("has_crc", True)  # Resolve CRC usage preference.
    impl_head = args.impl_head if args.impl_head is not None else inferred.get("impl_head", False)  # Resolve header mode preference.
    ldro_mode = args.ldro_mode if args.ldro_mode is not None else inferred.get("ldro_mode", 2)  # Decide on LDRO mode, defaulting to automatic.

    if sf is None or bw is None or samp_rate is None:
        raise ValueError("Spreading factor, bandwidth and sample rate must be provided")  # Ensure critical parameters have been resolved.

    iq = np.fromfile(file_path, dtype=np.complex64)  # Load the raw IQ samples from the capture file.
    if args.limit is not None:
        iq = iq[:args.limit]  # Truncate the sample set if the user requested a limit.

    receiver = OfflineLoraReceiver(
        iq_samples=iq,  # Feed the loaded IQ samples to the decoder flowgraph.
        samp_rate=samp_rate,  # Pass the resolved sample rate.
        bw=bw,  # Supply the bandwidth.
        sf=sf,  # Supply the spreading factor.
        cr=cr,  # Supply the coding rate.
        has_crc=has_crc,  # Inform the flowgraph whether CRCs are expected.
        impl_head=impl_head,  # Provide the implicit/explicit header selection.
        ldro_mode=ldro_mode,  # Pass the LDRO setting.
        sync_word=[args.sync_word],  # Wrap the sync word into a list as required by the block interface.
        pay_len=args.pay_len,  # Provide the maximum payload length.
    )

    receiver.run()  # Execute the flowgraph to process the stored samples.
    payload_bytes, tags, messages = receiver.results()  # Collect payload bytes, stream tags, and message-port outputs.
    frames = extract_frames(payload_bytes, tags)  # Break the concatenated payload into individual frames with metadata.
    combine_messages(frames, messages)  # Associate each frame with any textual messages produced by the receiver.

    if not frames:
        print("No frames were decoded")  # Inform the user when the capture did not yield any frames.
        return  # Exit early when nothing was decoded.

    for frame in frames:
        crc_state = "unknown"  # Start with an unknown CRC state for display.
        if frame.crc_valid is True:
            crc_state = "valid"  # Report valid CRC results explicitly.
        elif frame.crc_valid is False:
            crc_state = "invalid"  # Report failed CRC checks.
        elif frame.has_crc is False:
            crc_state = "not present"  # Indicate when CRCs were not part of the frame.
        print(
            f"Frame {frame.index}: {len(frame.payload)} bytes, CRC {crc_state}\n"
            f"  Hex: {frame.hex_payload()}"
        )  # Display a summary of the frame and its payload bytes in hexadecimal.
        if frame.message_text is not None:
            print(f"  Text: {frame.message_text}")  # Show the decoded textual payload when available.


if __name__ == "__main__":
    main()  # Execute the command-line interface when run as a script.
