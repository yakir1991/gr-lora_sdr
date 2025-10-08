#!/usr/bin/env python3  # Allow running the script directly with the Python 3 interpreter.
# -*- coding: utf-8 -*-  # Declare the file encoding to safely handle UTF-8 characters.

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: Tx Rx Simulation
# Author: Tapparel Joachim@EPFL,TCL
# GNU Radio version: 3.10.12.0

from gnuradio import blocks  # Import GNU Radio basic signal processing blocks.
import pmt  # Import Polymorphic Type support for message passing in GNU Radio.
from gnuradio import channels  # Import channel modeling blocks for simulating RF links.
from gnuradio.filter import firdes  # Import filter design helpers (auto-generated dependency).
from gnuradio import gr  # Import GNU Radio base classes such as top_block.
from gnuradio.fft import window  # Import FFT window definitions (auto-generated dependency).
import sys  # Access system-specific parameters and functions.
import signal  # Handle POSIX signals for graceful termination.
from argparse import ArgumentParser  # Parse command-line arguments (auto-generated dependency).
from gnuradio.eng_arg import eng_float, intx  # Provide engineering notation argument helpers.
from gnuradio import eng_notation  # Format numbers in engineering notation (auto-generated dependency).
import gnuradio.lora_sdr as lora_sdr  # Import the custom LoRa SDR blocks used in this project.
import threading  # Coordinate between threads to signal flowgraph readiness.




class tx_rx_simulation(gr.top_block):  # Define a top-level GNU Radio flowgraph for LoRa TX/RX simulation.

    def __init__(self):  # Initialize the flowgraph and configure its components.
        gr.top_block.__init__(self, "Tx Rx Simulation", catch_exceptions=True)  # Create the top_block with a name and exception handling.
        self.flowgraph_started = threading.Event()  # Use an Event to mark when the flowgraph is running.

        ##################################################
        # Variables
        ##################################################
        self.soft_decoding = soft_decoding = False  # Disable soft-decision decoding by default.
        self.sf = sf = 8  # Use spreading factor 8 for LoRa modulation.
        self.samp_rate = samp_rate = 125000  # Operate the simulation at a 125 kHz sample rate.
        self.preamb_len = preamb_len = 8  # Configure an 8-symbol preamble length.
        self.pay_len = pay_len = 16  # Set the payload length to 16 bytes.
        self.ldro = ldro = False  # Disable low data rate optimization (LDRO) initially.
        self.impl_head = impl_head = False  # Assume an explicit LoRa header by default.
        self.has_crc = has_crc = True  # Enable CRC generation and checking for payload integrity.
        self.cr = cr = 4  # Use coding rate 4 (i.e., 4/8) for forward error correction.
        self.clk_offset = clk_offset = 0  # Assume no transmitter/receiver clock offset.
        self.center_freq = center_freq = 868.1e6  # Target the 868.1 MHz ISM band center frequency.
        self.bw = bw = 125000  # Configure LoRa bandwidth at 125 kHz.
        self.SNRdB = SNRdB = 100  # Start with a very high SNR to emulate an ideal channel.

        ##################################################
        # Blocks
        ##################################################

        self.lora_sdr_whitening_0 = lora_sdr.whitening(False,False,',','packet_len')  # Scramble payload bits to avoid long runs of identical values.
        self.lora_sdr_modulate_0 = lora_sdr.modulate(sf, samp_rate, bw, [0x12], (int(20*2**sf*samp_rate/bw)),preamb_len)  # Generate a LoRa waveform from symbol data.
        self.lora_sdr_interleaver_0 = lora_sdr.interleaver(cr, sf, ldro, 125000)  # Interleave coded bits to combat burst errors.
        self.lora_sdr_header_decoder_0 = lora_sdr.header_decoder(impl_head, cr, pay_len, has_crc, ldro, True)  # Recover header metadata from the received frame.
        self.lora_sdr_header_0 = lora_sdr.header(impl_head, has_crc, cr)  # Build the LoRa PHY header with the current settings.
        self.lora_sdr_hamming_enc_0 = lora_sdr.hamming_enc(cr, sf)  # Apply Hamming forward-error correction to header bits.
        self.lora_sdr_hamming_dec_0 = lora_sdr.hamming_dec(soft_decoding)  # Decode Hamming-coded bits using hard or soft decisions.
        self.lora_sdr_gray_mapping_0 = lora_sdr.gray_mapping( soft_decoding)  # Map demodulated symbols into Gray-coded soft/hard bits.
        self.lora_sdr_gray_demap_0 = lora_sdr.gray_demap(sf)  # Convert encoded bits into the symbol order used by the modulator.
        self.lora_sdr_frame_sync_0 = lora_sdr.frame_sync(int(center_freq), bw, sf, impl_head, [18], (int(samp_rate/bw)),preamb_len)  # Align the receiver to the incoming LoRa frame timing.
        self.lora_sdr_fft_demod_0 = lora_sdr.fft_demod( soft_decoding, False)  # Demodulate LoRa chirps via FFT processing.
        self.lora_sdr_dewhitening_0 = lora_sdr.dewhitening()  # Reverse the scrambling applied at the transmitter.
        self.lora_sdr_deinterleaver_0 = lora_sdr.deinterleaver( soft_decoding)  # Undo the interleaving performed before modulation.
        self.lora_sdr_crc_verif_0 = lora_sdr.crc_verif( 1, False)  # Check the payload CRC to validate frame integrity.
        self.lora_sdr_add_crc_0 = lora_sdr.add_crc(has_crc)  # Attach a CRC to outgoing payloads when requested.
        # Use project-relative vectors path instead of absolute
        self.file_sink_tx_iq = blocks.file_sink(gr.sizeof_gr_complex*1, 'vectors/sf8_cr48_iq.bin', False)  # Record the transmitted complex IQ samples to disk.
        self.file_sink_tx_iq.set_unbuffered(False)  # Keep default buffering so disk writes remain efficient.
        self.file_sink_rx_payload = blocks.file_sink(gr.sizeof_char*1, '/tmp/lora_rx_payload.bin', False)  # Save decoded payload bytes for inspection.
        self.file_sink_rx_payload.set_unbuffered(False)  # Allow buffering for the payload dump file.
        self.channels_channel_model_0 = channels.channel_model(  # Create a simulated channel impairing the transmitted waveform.
            noise_voltage=(10**(-SNRdB/20)),  # Configure the noise level according to the desired SNR.
            frequency_offset=(center_freq*clk_offset*1e-6/samp_rate),  # Inject a carrier frequency offset based on oscillator mismatch.
            epsilon=(1.0+clk_offset*1e-6),  # Apply a sampling rate offset to emulate clock drift.
            taps=[1.0 + 0.0j],  # Model a flat channel with a single complex tap.
            noise_seed=0,  # Use a deterministic seed so runs are reproducible.
            block_tags=True)  # Preserve stream tags passing through the channel block.
        self.channels_channel_model_0.set_min_output_buffer((int(2**sf*samp_rate/bw*1.1)))  # Increase buffer size to accommodate an entire LoRa frame.
        self.blocks_file_source_0_0 = blocks.file_source(gr.sizeof_char*1, 'vectors/sf8_cr48_payload.bin', False, 0, 0)  # Read payload bytes from a test vector file.
        self.blocks_file_source_0_0.set_begin_tag(pmt.PMT_NIL)  # Start the file source without emitting a stream tag.


        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.lora_sdr_header_decoder_0, 'frame_info'), (self.lora_sdr_frame_sync_0, 'frame_info'))  # Forward decoded frame metadata to the synchronizer for re-locking.
        self.connect((self.blocks_file_source_0_0, 0), (self.lora_sdr_whitening_0, 0))  # Feed test payload bytes into the whitening stage.
        self.connect((self.channels_channel_model_0, 0), (self.lora_sdr_frame_sync_0, 0))  # Provide the impaired signal to the receiver synchronization block.
        self.connect((self.lora_sdr_add_crc_0, 0), (self.lora_sdr_hamming_enc_0, 0))  # Protect the header with Hamming coding after appending the CRC.
        self.connect((self.lora_sdr_crc_verif_0, 0), (self.file_sink_rx_payload, 0))  # Dump successfully decoded payload bytes to disk.
        self.connect((self.lora_sdr_deinterleaver_0, 0), (self.lora_sdr_hamming_dec_0, 0))  # Pass deinterleaved bits into the Hamming decoder.
        self.connect((self.lora_sdr_dewhitening_0, 0), (self.lora_sdr_crc_verif_0, 0))  # Verify integrity once the payload bits are descrambled.
        self.connect((self.lora_sdr_fft_demod_0, 0), (self.lora_sdr_gray_mapping_0, 0))  # Convert FFT demodulated symbols into Gray-coded bits.
        self.connect((self.lora_sdr_frame_sync_0, 0), (self.lora_sdr_fft_demod_0, 0))  # Route synchronized samples into the FFT demodulator.
        self.connect((self.lora_sdr_gray_demap_0, 0), (self.lora_sdr_modulate_0, 0))  # Provide mapped symbols to the LoRa modulator for waveform synthesis.
        self.connect((self.lora_sdr_gray_mapping_0, 0), (self.lora_sdr_deinterleaver_0, 0))  # Prepare Gray-mapped bits for deinterleaving prior to decoding.
        self.connect((self.lora_sdr_hamming_dec_0, 0), (self.lora_sdr_header_decoder_0, 0))  # Deliver decoded header bits to recover metadata.
        self.connect((self.lora_sdr_hamming_enc_0, 0), (self.lora_sdr_interleaver_0, 0))  # Send encoded bits into the interleaver before modulation.
        self.connect((self.lora_sdr_header_0, 0), (self.lora_sdr_add_crc_0, 0))  # Start the transmit chain by constructing the header and adding a CRC.
        self.connect((self.lora_sdr_header_decoder_0, 0), (self.lora_sdr_dewhitening_0, 0))  # Forward decoded payload bits to be descrambled.
        self.connect((self.lora_sdr_interleaver_0, 0), (self.lora_sdr_gray_demap_0, 0))  # Reorder interleaved bits into Gray-coded symbols.
        self.connect((self.lora_sdr_modulate_0, 0), (self.channels_channel_model_0, 0))  # Transmit the generated waveform through the simulated channel.
        self.connect((self.lora_sdr_modulate_0, 0), (self.file_sink_tx_iq, 0))  # Simultaneously log the transmit signal for analysis.
        self.connect((self.lora_sdr_whitening_0, 0), (self.lora_sdr_header_0, 0))  # Feed whitened payload bytes into the header builder.


    def get_soft_decoding(self):  # Expose whether soft-decision decoding is enabled.
        return self.soft_decoding  # Provide the current soft_decoding flag value.

    def set_soft_decoding(self, soft_decoding):  # Update the soft_decoding configuration.
        self.soft_decoding = soft_decoding  # Store the caller-provided boolean flag.

    def get_sf(self):  # Report the active spreading factor.
        return self.sf  # Return the integer SF value.

    def set_sf(self, sf):  # Change the spreading factor used across the signal chain.
        self.sf = sf  # Cache the new spreading factor locally.
        self.lora_sdr_gray_demap_0.set_sf(self.sf)  # Propagate SF changes to the Gray demapper.
        self.lora_sdr_hamming_enc_0.set_sf(self.sf)  # Update the encoder so it aligns with the new SF.
        self.lora_sdr_interleaver_0.set_sf(self.sf)  # Ensure the interleaver uses the correct symbol size.

    def get_samp_rate(self):  # Retrieve the sample rate driving the flowgraph.
        return self.samp_rate  # Return the stored sample rate value.

    def set_samp_rate(self, samp_rate):  # Adjust the sample rate used for processing.
        self.samp_rate = samp_rate  # Persist the new sample rate setting.
        self.channels_channel_model_0.set_frequency_offset((self.center_freq*self.clk_offset*1e-6/self.samp_rate))  # Recompute the frequency offset because it depends on sample rate.

    def get_preamb_len(self):  # Report the configured preamble length.
        return self.preamb_len  # Provide the number of preamble symbols.

    def set_preamb_len(self, preamb_len):  # Update the LoRa preamble length.
        self.preamb_len = preamb_len  # Save the caller-defined preamble symbol count.

    def get_pay_len(self):  # Return the payload length in bytes.
        return self.pay_len  # Provide the stored payload size.

    def set_pay_len(self, pay_len):  # Modify the payload length used for encoding and decoding.
        self.pay_len = pay_len  # Record the updated payload size.

    def get_ldro(self):  # Expose whether low data rate optimization is enabled.
        return self.ldro  # Return the LDRO boolean flag.

    def set_ldro(self, ldro):  # Change the LDRO configuration.
        self.ldro = ldro  # Store the new LDRO setting.

    def get_impl_head(self):  # Indicate whether an implicit header is used.
        return self.impl_head  # Provide the implicit header flag.

    def set_impl_head(self, impl_head):  # Update whether headers are implicit or explicit.
        self.impl_head = impl_head  # Save the provided implicit-header flag.

    def get_has_crc(self):  # Reveal if CRC protection is active.
        return self.has_crc  # Return the CRC-enabled flag.

    def set_has_crc(self, has_crc):  # Toggle CRC generation and checking.
        self.has_crc = has_crc  # Update the CRC usage boolean.

    def get_cr(self):  # Provide the current coding rate value.
        return self.cr  # Return the integer coding rate setting.

    def set_cr(self, cr):  # Adjust the coding rate used by the LoRa chain.
        self.cr = cr  # Store the new coding rate parameter.
        self.lora_sdr_hamming_enc_0.set_cr(self.cr)  # Keep the Hamming encoder in sync with the coding rate.
        self.lora_sdr_header_0.set_cr(self.cr)  # Update the header generator to reflect the new coding rate.
        self.lora_sdr_interleaver_0.set_cr(self.cr)  # Ensure the interleaver uses the correct redundancy settings.

    def get_clk_offset(self):  # Report the simulated clock offset in parts-per-million.
        return self.clk_offset  # Provide the stored clock offset value.

    def set_clk_offset(self, clk_offset):  # Modify the simulated oscillator mismatch.
        self.clk_offset = clk_offset  # Save the new clock offset magnitude.
        self.channels_channel_model_0.set_frequency_offset((self.center_freq*self.clk_offset*1e-6/self.samp_rate))  # Reapply the frequency offset derived from the clock error.
        self.channels_channel_model_0.set_timing_offset((1.0+self.clk_offset*1e-6))  # Adjust the timing offset for the same mismatch.

    def get_center_freq(self):  # Expose the tuned center frequency in Hertz.
        return self.center_freq  # Return the configured RF center frequency.

    def set_center_freq(self, center_freq):  # Update the simulated RF center frequency.
        self.center_freq = center_freq  # Persist the provided frequency value.
        self.channels_channel_model_0.set_frequency_offset((self.center_freq*self.clk_offset*1e-6/self.samp_rate))  # Recalculate frequency offset relative to the new center.

    def get_bw(self):  # Return the LoRa signal bandwidth.
        return self.bw  # Provide the stored bandwidth value.

    def set_bw(self, bw):  # Modify the LoRa bandwidth setting.
        self.bw = bw  # Record the new bandwidth selection.

    def get_SNRdB(self):  # Report the simulated signal-to-noise ratio in dB.
        return self.SNRdB  # Return the numeric SNR value.

    def set_SNRdB(self, SNRdB):  # Adjust the SNR applied by the channel model.
        self.SNRdB = SNRdB  # Save the new SNR configuration.
        self.channels_channel_model_0.set_noise_voltage((10**(-self.SNRdB/20)))  # Update the channel noise voltage to match the requested SNR.




def main(top_block_cls=tx_rx_simulation, options=None):  # Provide a command-line entry point for running the flowgraph.
    tb = top_block_cls()  # Instantiate the chosen top block implementation.

    def sig_handler(sig=None, frame=None):  # Respond to termination signals by shutting down cleanly.
        tb.stop()  # Request the flowgraph to halt execution.
        tb.wait()  # Block until all flowgraph threads exit.

        sys.exit(0)  # Exit the Python interpreter after cleanup finishes.

    signal.signal(signal.SIGINT, sig_handler)  # Trap Ctrl+C interrupts for graceful shutdown.
    signal.signal(signal.SIGTERM, sig_handler)  # Catch termination signals from the OS or supervisor.

    tb.start()  # Launch the GNU Radio scheduler threads.
    tb.flowgraph_started.set()  # Indicate to other threads that the flowgraph is running.

    try:  # Wait for user input before stopping, keeping the simulation active.
        input('Press Enter to quit: ')  # Prompt the user to terminate the program interactively.
    except EOFError:  # Handle non-interactive environments where stdin is closed.
        pass  # Ignore the error and proceed to stop the flowgraph.
    tb.stop()  # Request that the flowgraph cease processing.
    tb.wait()  # Ensure all processing threads have finished before exiting.


if __name__ == '__main__':  # Execute the main routine when the script runs as a program.
    main()  # Invoke the entry point to start the simulation.
