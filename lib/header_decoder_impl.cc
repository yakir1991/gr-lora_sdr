#ifdef HAVE_CONFIG_H  // Include the generated configuration header when the build system provides one.
#include "config.h"  // Bring in compile-time options such as feature toggles and platform specifics.
#endif  // End conditional inclusion of config.h.

#include <gnuradio/io_signature.h>  // Access GNU Radio helper classes for describing block input and output signatures.
#include "header_decoder_impl.h"  // Include the matching header that declares the header_decoder_impl class.
#include <gnuradio/lora_sdr/utilities.h>  // Pull in utility helpers shared across the LoRa SDR module.

namespace gr  // Start the GNU Radio namespace scope.
{
    namespace lora_sdr  // Enter the LoRa SDR namespace containing custom signal-processing blocks.
    {

        header_decoder::sptr  // Specify that the factory returns a shared pointer to a header_decoder interface.
        header_decoder::make(bool impl_head, uint8_t cr, uint32_t pay_len, bool has_crc, uint8_t ldro_mode, bool print_header)  // Define the public factory helper with configuration parameters for the decoder.
        {
            return gnuradio::get_initial_sptr(new header_decoder_impl(impl_head, cr, pay_len, has_crc, ldro_mode, print_header));  // Construct a new implementation object and wrap it in a GNU Radio smart pointer.
        }

        /*
     * The private constructor
     */
        header_decoder_impl::header_decoder_impl(bool impl_head, uint8_t cr, uint32_t pay_len, bool has_crc, uint8_t ldro_mode, bool print_header)  // Implement the constructor that stores operating parameters for later use.
            : gr::block("header_decoder",  // Call the base class constructor naming the block "header_decoder" for identification in flowgraphs.
                        gr::io_signature::make(1, 1, sizeof(uint8_t)),  // Describe a single input stream of uint8_t symbols.
                        gr::io_signature::make(1, 1, sizeof(uint8_t)))  // Describe a single output stream of uint8_t symbols.
        {
            m_impl_header = impl_head;  // Remember whether the incoming frames use an implicit header.
            m_print_header = print_header;  // Store whether the decoder should print header information to the console for debugging.
            m_cr = cr;  // Cache the coding rate used when interpreting parity bits.
            m_payload_len = pay_len;  // Record the payload length for implicit header scenarios.
            m_has_crc = has_crc;  // Record whether frames carry a CRC, affecting how many bytes belong to payload vs CRC.
            m_ldro_mode = ldro_mode;  // Store the low-data-rate optimization mode value for downstream consumers.

            pay_cnt = 0;  // Initialize the counter tracking how many payload symbols have been passed through.

            set_tag_propagation_policy(TPP_DONT);  // Disable automatic tag forwarding so the block can manage metadata manually.
            message_port_register_out(pmt::mp("frame_info"));  // Register an asynchronous message port used to publish decoded header information.


        }
        /*
     * Our virtual destructor.
     */
        header_decoder_impl::~header_decoder_impl()  // Default destructor since no explicit cleanup is required.
        {
        }

        void  // Indicate the forecast method returns void.
        header_decoder_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required)  // Inform the scheduler about the number of input symbols needed to produce requested output.
        {
            ninput_items_required[0] = noutput_items;  // Require an equal number of input items as the amount of output requested.
        }

        void header_decoder_impl::publish_frame_info(int cr, int pay_len, int crc, uint8_t ldro_mode, int err)  // Helper used to publish header metadata downstream and via the message port.
        {

            pmt::pmt_t header_content = pmt::make_dict();  // Start with an empty PMT dictionary that will hold header fields.

            header_content = pmt::dict_add(header_content, pmt::intern("cr"), pmt::from_long(cr));  // Store the detected coding rate into the dictionary.
            header_content = pmt::dict_add(header_content, pmt::intern("pay_len"), pmt::from_long(pay_len));  // Save the payload length inferred from the header.
            header_content = pmt::dict_add(header_content, pmt::intern("crc"), pmt::from_long(crc));  // Store the CRC presence flag for downstream processing.
            header_content = pmt::dict_add(header_content, pmt::intern("ldro_mode"), pmt::from_long(ldro_mode));  // Record the LDRO mode indicated by the header or configuration.
            header_content = pmt::dict_add(header_content, pmt::intern("err"), pmt::from_long(err));  // Attach the error status so consumers know whether the header was valid.
            message_port_pub(pmt::intern("frame_info"), header_content);  // Publish the header dictionary on the asynchronous message port.
            if(!err) //don't propagate downstream that a frame was detected  // Only attach stream tags when a valid header was decoded.
                add_item_tag(0, nitems_written(0), pmt::string_to_symbol("frame_info"), header_content);  // Add a frame_info tag to the output stream aligned with the current write index.
        }

        int header_decoder_impl::general_work(int noutput_items,  // Override GNU Radio's main work function that processes input items and produces output.
                                              gr_vector_int &ninput_items,  // Provide access to the vector describing how many input items are available.
                                              gr_vector_const_void_star &input_items,  // Provide const pointers to the input buffers.
                                              gr_vector_void_star &output_items)  // Provide mutable pointers to the output buffers.
        {
            const uint8_t *in = (const uint8_t *)input_items[0];  // Treat the first input buffer as an array of bytes containing LoRa symbols.
            uint8_t *out = (uint8_t *)output_items[0];  // Treat the output buffer as writable bytes for forwarding payload data.

            int nitem_to_process = ninput_items[0];  // Start assuming we can process the full number of items currently available.

            std::vector<tag_t> tags;  // Temporary storage for metadata tags detected on the input stream.
            get_tags_in_window(tags, 0, 0, ninput_items[0], pmt::string_to_symbol("frame_info"));  // Fetch any frame_info tags in the window to understand frame boundaries.
            if (tags.size())  // Only proceed with tag handling if at least one tag was found.
            {
                if (tags[0].offset != nitems_read(0))  // Check whether the first tag is ahead of the current read position.
                {
                    nitem_to_process = tags[0].offset - nitems_read(0);  // Limit processing to stop right before the upcoming tag.
                }
                else  // The tag aligns with the current input sample so the frame metadata is immediately available.
                {
                    if (tags.size() >= 2)  // When a second tag exists, it marks the end of the frame region.
                    {
                        nitem_to_process = tags[1].offset - tags[0].offset;  // Restrict processing to the span between the two tags.
                    }
                    else  // Otherwise the frame extends across all currently available items.
                    {
                        nitem_to_process = ninput_items[0];  // Process the entire available window.
                    }
                    pmt::pmt_t err = pmt::string_to_symbol("error");  // Symbol used to signal missing dictionary entries when reading tag metadata.
                    is_header = pmt::to_bool(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("is_header"), err));  // Determine whether the tagged region corresponds to header bytes or payload.
                    // print("ac ishead: "<<is_header);  // Legacy debugging comment left in the original source.
                    if (is_header)  // When the tag indicates header content, reset payload counters.
                    {
                        pay_cnt = 0;  // Reset the payload counter so we start counting payload bytes after the header finishes.
                    }
                }
            }
            if (is_header && nitem_to_process < 5 && !m_impl_header) //ensure to have a full PHY header to process  // In explicit header mode we must wait for all 5 header bytes before decoding.
            {
                nitem_to_process = 0;  // Postpone processing until sufficient header bytes arrive.
            }
            if (is_header)  // Handle the case where the current region corresponds to header symbols.
            {
                if (m_impl_header)  // For implicit headers we already know all parameters.
                { //implicit header, all parameters should have been provided  // Continue the explanation inline.
                    publish_frame_info(m_cr, m_payload_len, m_has_crc, m_ldro_mode, 0);  // Emit the preconfigured header info with no error flag.

                    for (int i = 0; i < nitem_to_process; i++)  // Forward the available symbols as payload or CRC bytes.
                    {
                        //only output payload or CRC
                        out[i] = in[i];  // Copy bytes straight through since implicit mode does not strip header information.
                    }
                    consume_each(nitem_to_process);  // Consume the processed items from the input stream.
                    return nitem_to_process;  // Inform the scheduler about the number of output items produced.
                }

                if (nitem_to_process == 0)  // If we still do not have enough data, exit early to wait for more samples.
                {
                    return 0;  // Produce no output and request more input.
                }

                //we assume entire header (5 bytes) is present now. otherwise we would exit before

                uint16_t payload_len = ((in[0] << 4) + in[1]);  // Reconstruct the payload length from the high and low nibbles.
                int cr = (in[2] & 0b1110) >> 1;  // Extract the coding rate bits from the third header byte.
                bool has_crc = (in[2] & 0b0001);  // Extract the CRC present flag from the lowest bit.

                int head_crc = in[3];  // Grab the parity nibble stored in the fourth header byte.
                int checksum = (in[4] << 3) + 0b0011;  // Combine the checksum nibble with constant bits as defined by the LoRa spec.

                int c4 = (in[0] & 0b1000) >> 3 ^ (in[0] & 0b0100) >> 2 ^ (in[0] & 0b0010) >> 1 ^ (in[0] & 0b0001);  // Recompute the first parity bit using the same relations as encoding.
                int c3 = (in[0] & 0b1000) >> 3 ^ (in[1] & 0b1000) >> 3 ^ (in[1] & 0b0100) >> 2 ^ (in[1] & 0b0010) >> 1 ^ (in[2] & 0b0001);  // Recompute the second parity bit to validate the header.
                int c2 = (in[0] & 0b0100) >> 2 ^ (in[1] & 0b1000) >> 3 ^ (in[1] & 0b0001) ^ (in[2] & 0b1000) >> 3 ^ (in[2] & 0b0010) >> 1;  // Recompute the third parity bit for error detection.
                int c1 = (in[0] & 0b0010) >> 1 ^ (in[1] & 0b0100) >> 2 ^ (in[1] & 0b0001) ^ (in[2] & 0b0100) >> 2 ^ (in[2] & 0b0010) >> 1 ^ (in[2] & 0b0001);  // Recompute the fourth parity bit for the header checksum nibble.
                int c0 = (in[0] & 0b0001) ^ (in[1] & 0b0010) >> 1 ^ (in[2] & 0b1000) >> 3 ^ (in[2] & 0b0100) >> 2 ^ (in[2] & 0b0010) >> 1 ^ (in[2] & 0b0001);  // Recompute the final parity bit.

                int parity_diff = ((c3 << 3) | (c2 << 2) | (c1 << 1) | c0) ^ (checksum & 0x0F);  // Compare computed parity bits against the received checksum nibble.
                int header_crc = (c4 << 1) | head_crc;  // Combine computed MSB parity with transmitted parity to evaluate CRC condition.

                //only send downstream if both the header parity and the header cyclic redundancy check matched
                if (!parity_diff && (header_crc == 2))  // Accept the header when both parity bits and CRC bits match specification.
                {
                    publish_frame_info(cr, payload_len, has_crc, checksum & 0x01, 0);  // Publish the decoded header fields with no error.
                    if (m_print_header)  // Optionally print header details for debugging when enabled.
                    {
                        printf("Len %u CR %d CRC %d LDRO %d\n", payload_len, cr, has_crc, checksum & 0x01);  // Write header parameters to stdout for inspection.
                    }

                    if (has_crc)  // If the payload carries a CRC, set the per-frame CRC status accordingly.
                    {
                        m_has_crc = true;  // Indicate that a CRC section should be expected at the end of the payload.
                        m_crc_ok = false;  // Reset the CRC validity flag until computed later.
                    }
                    else  // Otherwise no CRC bytes are present.
                    {
                        m_has_crc = false;  // Clear the CRC flag so downstream knows only payload bytes follow.
                        m_crc_ok = false;  // Reset the CRC flag even though it will remain unused.
                    }

                    is_header = false;  // Mark that header processing has completed so subsequent bytes are payload.
                }
                else  // Header parity check failed, meaning the header is invalid or corrupted.
                {
                    publish_frame_info(cr, payload_len, has_crc, checksum & 0x01, 1);  // Publish the decoded values but flag an error so downstream can react.
                    if (m_print_header)  // If printing is enabled, notify about the failure.
                    {
                        printf("Header decoding error\n");  // Print a diagnostic message indicating header corruption.
                    }
                    is_header = false;  // Drop header mode to avoid repeatedly reprocessing the same corrupted bytes.
                }

                consume_each(5);  // Consume the five header bytes that were read from the input stream.
                return 0;  // Produce no output because header bytes are not forwarded downstream.
            }

            for (int i = 0; i < nitem_to_process; i++)  // Forward payload or CRC bytes when not in header mode.
            {
                out[i] = in[i];  // Copy the input byte directly to the output buffer.
            }

            consume_each(nitem_to_process);  // Inform the scheduler how many input bytes were consumed in this pass.
            return nitem_to_process;  // Report the number of output bytes generated.
        }

    } /* namespace lora_sdr */  // Close the LoRa SDR namespace scope.
} /* namespace gr */  // Close the GNU Radio namespace scope.
