# Reception Chain

## Overview
The LoRa SDR reception chain is assembled as a GNU Radio hierarchical block that links the custom synchronization, demodulation, channel decoding, and integrity-check stages in sequence. The hierarchy accepts a complex baseband stream and produces both a byte stream and asynchronous messages once payloads are verified.【F:python/lora_sdr/lora_sdr_lora_rx.py†L18-L71】

## Signal Flow at a Glance
1. **Frame synchronization** – locks to the preamble, corrects carrier and timing offsets, and forwards aligned symbols.【F:lib/frame_sync_impl.cc†L808-L1144】
2. **FFT-based demodulation** – dechirps the aligned symbol, performs an FFT, and extracts either hard decisions or soft metrics.【F:lib/fft_demod_impl.cc†L31-L137】
3. **Gray demapping** – converts the demodulated indices into Gray-coded symbols or passes soft likelihoods unchanged.【F:lib/gray_mapping_impl.cc†L34-L137】
4. **Deinterleaving** – reverses LoRa’s diagonal interleaver for either hard bits or soft LLRs while propagating frame metadata.【F:lib/deinterleaver_impl.cc†L34-L178】
5. **Hamming decoding** – applies the “4/x” Hamming code, handling both soft-input ML decoding and hard lookups.【F:lib/hamming_dec_impl.cc†L44-L192】
6. **Header decoding** – interprets or injects PHY header parameters and republishes them as stream tags and messages.【F:lib/header_decoder_impl.cc†L39-L220】
7. **Dewhitening** – reverses the LoRa whitening sequence on the payload while preserving CRC bytes if present.【F:lib/dewhitening_impl.cc†L41-L199】
8. **CRC verification & output** – buffers bytes until a full payload (and CRC) is ready, validates integrity, and emits the decoded message.【F:lib/crc_verif_impl.cc†L150-L335】

The following sections document every stage in detail, including the implementation files, noteworthy code paths, buffer usage, and whether carrier-frequency offset (CFO) or symbol-timing offset (STO) compensation takes place.

## Stage Details

### 1. Frame Synchronization and Front-End Conditioning
* **Implementation:** `lib/frame_sync_impl.cc` constructs extensive working buffers (preamble storage, correction vectors, downsampled symbols) once it knows the spreading factor and oversampling ratio.【F:lib/frame_sync_impl.cc†L100-L154】
* **Core processing:** The `general_work` method down-samples the oversampled stream, detects preamble up-chirps, and orchestrates a state machine that captures preamble, network identifier, and down-chirp symbols.【F:lib/frame_sync_impl.cc†L808-L1108】
* **Buffers:** During detection, the block continuously copies samples into `preamble_raw`, `preamble_raw_up`, and `net_id_samp`, effectively acting as software buffers that hold multiple symbols for offset estimation and later reuse.【F:lib/frame_sync_impl.cc†L918-L1108】
* **CFO/STO handling:** Fractional CFO is estimated by dechirping and FFT-processing the stored preamble (`estimate_CFO_frac_Bernier`) and applied via complex exponential correction vectors, while fractional STO is estimated similarly to align the sampling phase.【F:lib/frame_sync_impl.cc†L217-L520】【F:lib/frame_sync_impl.cc†L984-L1114】 Integer CFO (equivalent to coarse STO) is derived from down-chirps and corrected by rotating the buffered preamble symbols before symbol decisions continue.【F:lib/frame_sync_impl.cc†L1094-L1144】
* **Metadata:** Frame boundary tags drive dynamic configuration of downstream spreading factor and coding settings so later stages only process samples that belong to the current frame.【F:lib/frame_sync_impl.cc†L854-L888】

### 2. FFT-Based Demodulation
* **Implementation:** `lib/fft_demod_impl.cc` prepares per-symbol dechirped arrays and FFT workspaces whenever the spreading factor changes, sizing buffers like `m_fft`, `m_dechirped`, and `m_fft_mag_sq` accordingly.【F:lib/fft_demod_impl.cc†L31-L119】
* **Core processing:** Each call multiplies the incoming symbol by an ideal down-chirp, runs a KISS FFT, and converts the magnitude squared spectrum into either the argmax (hard symbol) or per-bin log-likelihood ratios.【F:lib/fft_demod_impl.cc†L90-L137】
* **Buffers:** The block uses dynamically allocated FFT input/output arrays and a heap-allocated magnitude buffer per symbol; these act as temporary buffers but are freed after use.【F:lib/fft_demod_impl.cc†L90-L118】
* **CFO/STO:** No additional CFO or STO correction is attempted here; the demodulator relies on the synchronized stream provided by the previous stage.

### 3. Gray Demapping
* **Implementation:** `lib/gray_mapping_impl.cc` operates as a synchronous block that inspects `frame_info` tags to track whether the current symbols correspond to headers and to adjust the expected spreading factor.【F:lib/gray_mapping_impl.cc†L34-L110】
* **Processing & buffers:** In soft-decoding mode the block simply copies the LLR buffer through unchanged (acting as a pass-through buffer), while in hard-decoding mode it performs an in-place Gray demap on each symbol value.【F:lib/gray_mapping_impl.cc†L112-L137】 No additional CFO/STO corrections occur at this point.

### 4. Deinterleaving
* **Implementation:** `lib/deinterleaver_impl.cc` uses the frame tags to learn whether the stream carries header or payload codewords and to derive the effective spreading factor and coding rate before processing begins.【F:lib/deinterleaver_impl.cc†L80-L119】
* **Buffers:** For soft decoding, two-dimensional vectors (`inter_bin` and `deinter_bin`) buffer entire codeword matrices so the algorithm can undo the diagonal interleaving; the hard-decision path builds similar boolean matrices.【F:lib/deinterleaver_impl.cc†L123-L183】
* **Processing:** After populating the matrices, the code walks the diagonal pattern to relocate bits/LLRs into the correct order and writes them back to the output buffer.【F:lib/deinterleaver_impl.cc†L135-L163】 No CFO/STO work is performed.

### 5. Hamming Decoding
* **Implementation:** `lib/hamming_dec_impl.cc` reads `frame_info` tags to determine the current coding rate and whether the bytes belong to the header, ensuring the decoder uses the right codeword length.【F:lib/hamming_dec_impl.cc†L80-L118】
* **Buffers:** In soft mode it copies each codeword into a temporary `codeword_LLR` vector and evaluates all possible Hamming codewords, while the hard path constructs boolean vectors from the buffered symbol values.【F:lib/hamming_dec_impl.cc†L120-L192】
* **Processing:** The decoder scores each possible codeword using the buffered LLRs, picks the maximum-likelihood entry, and outputs the recovered data nibble. CFO/STO adjustments are not part of this stage.

### 6. Header Decoding and Metadata Publication
* **Implementation:** `lib/header_decoder_impl.cc` instantiates a general block that disables automatic tag propagation so it can manage tags explicitly and registers a message port for broadcasting decoded header information.【F:lib/header_decoder_impl.cc†L39-L116】
* **Processing:** The block inspects `frame_info` tags to separate header bytes from payload and branches on the configured header mode. In explicit mode it stalls until five header bytes are available, validates them, and republishes the decoded parameters as tags and messages for later stages.【F:lib/header_decoder_impl.cc†L191-L276】
* **No-header (implicit) behaviour:** When `impl_head` is true and no PHY header is transmitted, the decoder reuses the constructor parameters (`m_cr`, `m_payload_len`, `m_has_crc`, `m_ldro_mode`) to publish metadata immediately and simply copies all received nibbles as payload or CRC data.【F:lib/header_decoder_impl.cc†L39-L67】【F:lib/header_decoder_impl.cc†L203-L222】
* **Buffers:** Data is read directly from the GNU Radio input buffer; no long-lived auxiliary buffer is necessary beyond counters. CFO/STO corrections are not performed.

### 7. Dewhitening
* **Implementation:** `lib/dewhitening_impl.cc` registers message handlers to reset its internal state on new frames and to receive payload length and CRC presence updates.【F:lib/dewhitening_impl.cc†L74-L149】
* **Buffers:** The block walks the incoming byte stream two nibbles at a time, XOR-ing against the `whitening_seq` table while appending results to the `dewhitened` vector, which acts as the payload buffer.【F:lib/dewhitening_impl.cc†L160-L199】 CRC bytes are optionally copied without modification when whitening should be skipped.【F:lib/dewhitening_impl.cc†L178-L186】
* **CFO/STO:** None; timing corrections happen earlier.

### 8. CRC Verification and Final Output
* **Implementation:** `lib/crc_verif_impl.cc` gathers payload metadata from tags, appends incoming bytes to an internal buffer, and only proceeds once the payload (and CRC) is complete.【F:lib/crc_verif_impl.cc†L150-L205】
* **Buffers:** The `in_buff` vector accumulates bytes until CRC checks can be made; once validated, the block publishes the decoded message string and trims the buffer to prepare for the next frame.【F:lib/crc_verif_impl.cc†L179-L335】
* **Processing:** When a CRC is present the block recomputes it over the payload, compares it with the appended checksum, annotates the stream with a `crc_valid` tag, and optionally prints the payload. Without a CRC it still forwards the buffered payload and metadata.【F:lib/crc_verif_impl.cc†L192-L349】 No CFO/STO handling is needed at this stage.

## Headerless (Implicit) Mode End-to-End
* **Metadata seeding:** The hierarchical receiver passes `impl_head=True` together with the expected coding rate, payload length, CRC flag, and LDRO mode into both the synchronizer and header decoder when operating without a transmitted header.【F:python/lora_sdr/lora_sdr_lora_rx.py†L31-L55】
* **Frame synchronization:** The synchronizer stores the provided payload configuration and uses it to predict the number of symbols to forward, even though no header will appear on the stream.【F:lib/frame_sync_impl.cc†L704-L758】
* **Header stage:** Instead of decoding bytes, `header_decoder_impl` publishes the pre-configured metadata immediately and forwards the nibble stream unchanged so downstream blocks receive payload data without delay.【F:lib/header_decoder_impl.cc†L203-L222】
* **Downstream consumers:** Dewhitening and CRC verification still receive the correct `frame_info` tags from the implicit header path, allowing them to handle whitening, optional CRC bytes, and payload buffering exactly as in explicit mode.【F:lib/dewhitening_impl.cc†L120-L200】【F:lib/crc_verif_impl.cc†L157-L349】

## Configuration Matrix and Optional Paths
* **Soft vs. hard decoding:** The top-level block exposes a `soft_decoding` switch that configures the FFT demodulator, Gray mapper, deinterleaver, and Hamming decoder to work either with LLRs or hard symbol decisions.【F:python/lora_sdr/lora_sdr_lora_rx.py†L31-L55】【F:lib/fft_demod_impl.cc†L188-L260】【F:lib/gray_mapping_impl.cc†L112-L137】【F:lib/deinterleaver_impl.cc†L120-L183】【F:lib/hamming_dec_impl.cc†L120-L210】 Soft mode preserves floating-point likelihood buffers, while hard mode performs in-place Gray demapping and lookups.
* **CRC optionality:** `has_crc` determines whether CRC bytes are expected. When absent, the CRC verifier still buffers the payload but bypasses checksum recomputation and tag updates once the configured payload length is reached.【F:python/lora_sdr/lora_sdr_lora_rx.py†L31-L55】【F:lib/crc_verif_impl.cc†L192-L349】
* **Implicit vs. explicit headers:** The `impl_head` flag selects between the headerless flow above and the explicit decoding path that waits for a five-byte PHY header before publishing metadata.【F:python/lora_sdr/lora_sdr_lora_rx.py†L31-L55】【F:lib/header_decoder_impl.cc†L191-L276】
* **LDRO handling:** Frame synchronization either honors the configured LDRO mode or auto-enables it when symbol durations exceed the datasheet threshold, ensuring downstream timing stays consistent with the chosen option.【F:python/lora_sdr/lora_sdr_lora_rx.py†L31-L55】【F:lib/frame_sync_impl.cc†L704-L758】

## Notes on Buffering and Offset Correction Summary
* **Buffers are heavily used** in the synchronization, deinterleaving, whitening, and CRC stages to accumulate whole-symbol or whole-frame contexts before making decisions.【F:lib/frame_sync_impl.cc†L100-L154】【F:lib/deinterleaver_impl.cc†L123-L183】【F:lib/dewhitening_impl.cc†L160-L199】【F:lib/crc_verif_impl.cc†L179-L335】 The FFT demodulator also allocates temporary buffers per symbol.【F:lib/fft_demod_impl.cc†L90-L118】
* **CFO and STO corrections** occur exclusively in `frame_sync_impl`: fractional CFO/STO are estimated via FFT analysis of buffered preamble symbols, while integer CFO (coarse STO) is derived from down-chirp decisions and used to rotate stored samples before emission.【F:lib/frame_sync_impl.cc†L217-L520】【F:lib/frame_sync_impl.cc†L984-L1114】 Downstream stages assume the stream is already time- and frequency-aligned.

This detailed walkthrough should make it easier to trace samples through the GNU Radio-based LoRa reception chain, understand where buffering is required, and pinpoint the code responsible for synchronization and metadata handling.
