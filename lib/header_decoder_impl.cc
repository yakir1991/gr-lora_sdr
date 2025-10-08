// Only compile the following section when HAVE_CONFIG_H is defined. (inline comment notes: Include the generated configuration header when the build system provides one.)
#ifdef HAVE_CONFIG_H  // Include the generated configuration header when the build system provides one.
// Bring in declarations from "config.h". (inline comment notes: Bring in compile-time options such as feature toggles and platform specifics.)
#include "config.h"  // Bring in compile-time options such as feature toggles and platform specifics.
// Close the preceding conditional compilation block. (inline comment notes: End conditional inclusion of config.h.)
#endif  // End conditional inclusion of config.h.

// Bring in declarations from <gnuradio/io_signature.h>. (inline comment notes: Access GNU Radio helper classes for describing block input and output signatures.)
#include <gnuradio/io_signature.h>  // Access GNU Radio helper classes for describing block input and output signatures.
// Bring in declarations from "header_decoder_impl.h". (inline comment notes: Include the matching header that declares the header_decoder_impl class.)
#include "header_decoder_impl.h"  // Include the matching header that declares the header_decoder_impl class.
// Bring in declarations from <gnuradio/lora_sdr/utilities.h>. (inline comment notes: Pull in utility helpers shared across the LoRa SDR module.)
#include <gnuradio/lora_sdr/utilities.h>  // Pull in utility helpers shared across the LoRa SDR module.

// Start namespace gr scope. (inline comment notes: Start the GNU Radio namespace scope.)
namespace gr  // Start the GNU Radio namespace scope.
// Open a new scope block.
{
    // Start namespace lora_sdr scope. (inline comment notes: Enter the LoRa SDR namespace containing custom signal-processing blocks.)
    namespace lora_sdr  // Enter the LoRa SDR namespace containing custom signal-processing blocks.
    // Open a new scope block.
    {

        // Specify the shared pointer typedef associated with header_decoder. (inline comment notes: Specify that the factory returns a shared pointer to a header_decoder interface.)
        header_decoder::sptr  // Specify that the factory returns a shared pointer to a header_decoder interface.
        // Define the r::make function. (inline comment notes: Define the public factory helper with configuration parameters for the decoder.)
        header_decoder::make(bool impl_head, uint8_t cr, uint32_t pay_len, bool has_crc, uint8_t ldro_mode, bool print_header)  // Define the public factory helper with configuration parameters for the decoder.
        // Open a new scope block.
        {
            // Return gnuradio::get_initial_sptr(new header_decoder_impl(impl_head, cr, pay_len, has_crc, ldro_mode, print_header)) to the caller. (inline comment notes: Construct a new implementation object and wrap it in a GNU Radio smart pointer.)
            return gnuradio::get_initial_sptr(new header_decoder_impl(impl_head, cr, pay_len, has_crc, ldro_mode, print_header));  // Construct a new implementation object and wrap it in a GNU Radio smart pointer.
        // Close the current scope block.
        }

        /*
     * The private constructor
     */
        // Define the l::header_decoder_impl function. (inline comment notes: Implement the constructor that stores operating parameters for later use.)
        header_decoder_impl::header_decoder_impl(bool impl_head, uint8_t cr, uint32_t pay_len, bool has_crc, uint8_t ldro_mode, bool print_header)  // Implement the constructor that stores operating parameters for later use.
            // Initialize base classes or members with gr::block("header_decoder",. (inline comment notes: Call the base class constructor naming the block "header_decoder" for identification in flowgraphs.)
            : gr::block("header_decoder",  // Call the base class constructor naming the block "header_decoder" for identification in flowgraphs.
                        // Specify parameter or initializer gr::io_signature::make(1, 1, sizeof(uint8_t)). (inline comment notes: Describe a single input stream of uint8_t symbols.)
                        gr::io_signature::make(1, 1, sizeof(uint8_t)),  // Describe a single input stream of uint8_t symbols.
                        // Define the e::make function. (inline comment notes: Describe a single output stream of uint8_t symbols.)
                        gr::io_signature::make(1, 1, sizeof(uint8_t)))  // Describe a single output stream of uint8_t symbols.
        // Open a new scope block.
        {
            // Assign m_impl_header to impl_head. (inline comment notes: Remember whether the incoming frames use an implicit header.)
            m_impl_header = impl_head;  // Remember whether the incoming frames use an implicit header.
            // Assign m_print_header to print_header. (inline comment notes: Store whether the decoder should print header information to the console for debugging.)
            m_print_header = print_header;  // Store whether the decoder should print header information to the console for debugging.
            // Assign m_cr to cr. (inline comment notes: Cache the coding rate used when interpreting parity bits.)
            m_cr = cr;  // Cache the coding rate used when interpreting parity bits.
            // Assign m_payload_len to pay_len. (inline comment notes: Record the payload length for implicit header scenarios.)
            m_payload_len = pay_len;  // Record the payload length for implicit header scenarios.
            // Assign m_has_crc to has_crc. (inline comment notes: Record whether frames carry a CRC, affecting how many bytes belong to payload vs CRC.)
            m_has_crc = has_crc;  // Record whether frames carry a CRC, affecting how many bytes belong to payload vs CRC.
            // Assign m_ldro_mode to ldro_mode. (inline comment notes: Store the low-data-rate optimization mode value for downstream consumers.)
            m_ldro_mode = ldro_mode;  // Store the low-data-rate optimization mode value for downstream consumers.

            // Assign pay_cnt to 0. (inline comment notes: Initialize the counter tracking how many payload symbols have been passed through.)
            pay_cnt = 0;  // Initialize the counter tracking how many payload symbols have been passed through.

            // Call set_tag_propagation_policy with arguments (TPP_DONT). (inline comment notes: Disable automatic tag forwarding so the block can manage metadata manually.)
            set_tag_propagation_policy(TPP_DONT);  // Disable automatic tag forwarding so the block can manage metadata manually.
            // Call message_port_register_out with arguments (pmt::mp("frame_info")). (inline comment notes: Register an asynchronous message port used to publish decoded header information.)
            message_port_register_out(pmt::mp("frame_info"));  // Register an asynchronous message port used to publish decoded header information.


        // Close the current scope block.
        }
        /*
     * Our virtual destructor.
     */
        // Define the l::~header_decoder_impl function. (inline comment notes: Default destructor since no explicit cleanup is required.)
        header_decoder_impl::~header_decoder_impl()  // Default destructor since no explicit cleanup is required.
        // Open a new scope block.
        {
        // Close the current scope block.
        }

        // Execute void. (inline comment notes: Indicate the forecast method returns void.)
        void  // Indicate the forecast method returns void.
        // Define the l::forecast function. (inline comment notes: Inform the scheduler about the number of input symbols needed to produce requested output.)
        header_decoder_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required)  // Inform the scheduler about the number of input symbols needed to produce requested output.
        // Open a new scope block.
        {
            // Assign ninput_items_required[0] to noutput_items. (inline comment notes: Require an equal number of input items as the amount of output requested.)
            ninput_items_required[0] = noutput_items;  // Require an equal number of input items as the amount of output requested.
        // Close the current scope block.
        }

        // Define the l::publish_frame_info function. (inline comment notes: Helper used to publish header metadata downstream and via the message port.)
        void header_decoder_impl::publish_frame_info(int cr, int pay_len, int crc, uint8_t ldro_mode, int err)  // Helper used to publish header metadata downstream and via the message port.
        // Open a new scope block.
        {

            // Assign pmt::pmt_t header_content to pmt::make_dict(). (inline comment notes: Start with an empty PMT dictionary that will hold header fields.)
            pmt::pmt_t header_content = pmt::make_dict();  // Start with an empty PMT dictionary that will hold header fields.

            // Assign header_content to pmt::dict_add(header_content, pmt::intern("cr"), pmt::from_long(cr)). (inline comment notes: Store the detected coding rate into the dictionary.)
            header_content = pmt::dict_add(header_content, pmt::intern("cr"), pmt::from_long(cr));  // Store the detected coding rate into the dictionary.
            // Assign header_content to pmt::dict_add(header_content, pmt::intern("pay_len"), pmt::from_long(pay_len)). (inline comment notes: Save the payload length inferred from the header.)
            header_content = pmt::dict_add(header_content, pmt::intern("pay_len"), pmt::from_long(pay_len));  // Save the payload length inferred from the header.
            // Assign header_content to pmt::dict_add(header_content, pmt::intern("crc"), pmt::from_long(crc)). (inline comment notes: Store the CRC presence flag for downstream processing.)
            header_content = pmt::dict_add(header_content, pmt::intern("crc"), pmt::from_long(crc));  // Store the CRC presence flag for downstream processing.
            // Assign header_content to pmt::dict_add(header_content, pmt::intern("ldro_mode"), pmt::from_long(ldro_mode)). (inline comment notes: Record the LDRO mode indicated by the header or configuration.)
            header_content = pmt::dict_add(header_content, pmt::intern("ldro_mode"), pmt::from_long(ldro_mode));  // Record the LDRO mode indicated by the header or configuration.
            // Assign header_content to pmt::dict_add(header_content, pmt::intern("err"), pmt::from_long(err)). (inline comment notes: Attach the error status so consumers know whether the header was valid.)
            header_content = pmt::dict_add(header_content, pmt::intern("err"), pmt::from_long(err));  // Attach the error status so consumers know whether the header was valid.
            // Call message_port_pub with arguments (pmt::intern("frame_info"), header_content). (inline comment notes: Publish the header dictionary on the asynchronous message port.)
            message_port_pub(pmt::intern("frame_info"), header_content);  // Publish the header dictionary on the asynchronous message port.
            // Branch when condition (!err) evaluates to true. (inline comment notes: don't propagate downstream that a frame was detected  // Only attach stream tags when a valid header was decoded.)
            if(!err) //don't propagate downstream that a frame was detected  // Only attach stream tags when a valid header was decoded.
                // Call add_item_tag with arguments (0, nitems_written(0), pmt::string_to_symbol("frame_info"), header_content). (inline comment notes: Add a frame_info tag to the output stream aligned with the current write index.)
                add_item_tag(0, nitems_written(0), pmt::string_to_symbol("frame_info"), header_content);  // Add a frame_info tag to the output stream aligned with the current write index.
        // Close the current scope block.
        }

        // Specify parameter or initializer int header_decoder_impl::general_work(int noutput_items. (inline comment notes: Override GNU Radio's main work function that processes input items and produces output.)
        int header_decoder_impl::general_work(int noutput_items,  // Override GNU Radio's main work function that processes input items and produces output.
                                              // Specify parameter or initializer gr_vector_int &ninput_items. (inline comment notes: Provide access to the vector describing how many input items are available.)
                                              gr_vector_int &ninput_items,  // Provide access to the vector describing how many input items are available.
                                              // Specify parameter or initializer gr_vector_const_void_star &input_items. (inline comment notes: Provide const pointers to the input buffers.)
                                              gr_vector_const_void_star &input_items,  // Provide const pointers to the input buffers.
                                              // Execute gr_vector_void_star &output_items). (inline comment notes: Provide mutable pointers to the output buffers.)
                                              gr_vector_void_star &output_items)  // Provide mutable pointers to the output buffers.
        // Open a new scope block.
        {
            // Assign const uint8_t *in to (const uint8_t *)input_items[0]. (inline comment notes: Treat the first input buffer as an array of bytes containing LoRa symbols.)
            const uint8_t *in = (const uint8_t *)input_items[0];  // Treat the first input buffer as an array of bytes containing LoRa symbols.
            // Assign uint8_t *out to (uint8_t *)output_items[0]. (inline comment notes: Treat the output buffer as writable bytes for forwarding payload data.)
            uint8_t *out = (uint8_t *)output_items[0];  // Treat the output buffer as writable bytes for forwarding payload data.

            // Assign int nitem_to_process to ninput_items[0]. (inline comment notes: Start assuming we can process the full number of items currently available.)
            int nitem_to_process = ninput_items[0];  // Start assuming we can process the full number of items currently available.

            // Declare std::vector<tag_t> tags. (inline comment notes: Temporary storage for metadata tags detected on the input stream.)
            std::vector<tag_t> tags;  // Temporary storage for metadata tags detected on the input stream.
            // Call get_tags_in_window with arguments (tags, 0, 0, ninput_items[0], pmt::string_to_symbol("frame_info")). (inline comment notes: Fetch any frame_info tags in the window to understand frame boundaries.)
            get_tags_in_window(tags, 0, 0, ninput_items[0], pmt::string_to_symbol("frame_info"));  // Fetch any frame_info tags in the window to understand frame boundaries.
            // Branch when condition (tags.size()) evaluates to true. (inline comment notes: Only proceed with tag handling if at least one tag was found.)
            if (tags.size())  // Only proceed with tag handling if at least one tag was found.
            // Open a new scope block.
            {
                // Branch when condition (tags[0].offset != nitems_read(0)) evaluates to true. (inline comment notes: Check whether the first tag is ahead of the current read position.)
                if (tags[0].offset != nitems_read(0))  // Check whether the first tag is ahead of the current read position.
                // Open a new scope block.
                {
                    // Assign nitem_to_process to tags[0].offset - nitems_read(0). (inline comment notes: Limit processing to stop right before the upcoming tag.)
                    nitem_to_process = tags[0].offset - nitems_read(0);  // Limit processing to stop right before the upcoming tag.
                // Close the current scope block.
                }
                // Handle the alternative branch when previous conditions fail. (inline comment notes: The tag aligns with the current input sample so the frame metadata is immediately available.)
                else  // The tag aligns with the current input sample so the frame metadata is immediately available.
                // Open a new scope block.
                {
                    // Branch when condition (tags.size() >= 2) evaluates to true. (inline comment notes: When a second tag exists, it marks the end of the frame region.)
                    if (tags.size() >= 2)  // When a second tag exists, it marks the end of the frame region.
                    // Open a new scope block.
                    {
                        // Assign nitem_to_process to tags[1].offset - tags[0].offset. (inline comment notes: Restrict processing to the span between the two tags.)
                        nitem_to_process = tags[1].offset - tags[0].offset;  // Restrict processing to the span between the two tags.
                    // Close the current scope block.
                    }
                    // Handle the alternative branch when previous conditions fail. (inline comment notes: Otherwise the frame extends across all currently available items.)
                    else  // Otherwise the frame extends across all currently available items.
                    // Open a new scope block.
                    {
                        // Assign nitem_to_process to ninput_items[0]. (inline comment notes: Process the entire available window.)
                        nitem_to_process = ninput_items[0];  // Process the entire available window.
                    // Close the current scope block.
                    }
                    // Assign pmt::pmt_t err to pmt::string_to_symbol("error"). (inline comment notes: Symbol used to signal missing dictionary entries when reading tag metadata.)
                    pmt::pmt_t err = pmt::string_to_symbol("error");  // Symbol used to signal missing dictionary entries when reading tag metadata.
                    // Assign is_header to pmt::to_bool(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("is_header"), err)). (inline comment notes: Determine whether the tagged region corresponds to header bytes or payload.)
                    is_header = pmt::to_bool(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("is_header"), err));  // Determine whether the tagged region corresponds to header bytes or payload.
                    // print("ac ishead: "<<is_header);  // Legacy debugging comment left in the original source.
                    // Branch when condition (is_header) evaluates to true. (inline comment notes: When the tag indicates header content, reset payload counters.)
                    if (is_header)  // When the tag indicates header content, reset payload counters.
                    // Open a new scope block.
                    {
                        // Assign pay_cnt to 0. (inline comment notes: Reset the payload counter so we start counting payload bytes after the header finishes.)
                        pay_cnt = 0;  // Reset the payload counter so we start counting payload bytes after the header finishes.
                    // Close the current scope block.
                    }
                // Close the current scope block.
                }
            // Close the current scope block.
            }
            // Branch when condition (is_header && nitem_to_process < 5 && !m_impl_header) evaluates to true. (inline comment notes: ensure to have a full PHY header to process  // In explicit header mode we must wait for all 5 header bytes before decoding.)
            if (is_header && nitem_to_process < 5 && !m_impl_header) //ensure to have a full PHY header to process  // In explicit header mode we must wait for all 5 header bytes before decoding.
            // Open a new scope block.
            {
                // Assign nitem_to_process to 0. (inline comment notes: Postpone processing until sufficient header bytes arrive.)
                nitem_to_process = 0;  // Postpone processing until sufficient header bytes arrive.
            // Close the current scope block.
            }
            // Branch when condition (is_header) evaluates to true. (inline comment notes: Handle the case where the current region corresponds to header symbols.)
            if (is_header)  // Handle the case where the current region corresponds to header symbols.
            // Open a new scope block.
            {
                // Branch when condition (m_impl_header) evaluates to true. (inline comment notes: For implicit headers we already know all parameters.)
                if (m_impl_header)  // For implicit headers we already know all parameters.
                // Open a new scope block. (inline comment notes: implicit header, all parameters should have been provided  // Continue the explanation inline.)
                { //implicit header, all parameters should have been provided  // Continue the explanation inline.
                    // Call publish_frame_info with arguments (m_cr, m_payload_len, m_has_crc, m_ldro_mode, 0). (inline comment notes: Emit the preconfigured header info with no error flag.)
                    publish_frame_info(m_cr, m_payload_len, m_has_crc, m_ldro_mode, 0);  // Emit the preconfigured header info with no error flag.

                    // Iterate with loop parameters (int i = 0; i < nitem_to_process; i++). (inline comment notes: Forward the available symbols as payload or CRC bytes.)
                    for (int i = 0; i < nitem_to_process; i++)  // Forward the available symbols as payload or CRC bytes.
                    // Open a new scope block.
                    {
                        //only output payload or CRC
                        // Assign out[i] to in[i]. (inline comment notes: Copy bytes straight through since implicit mode does not strip header information.)
                        out[i] = in[i];  // Copy bytes straight through since implicit mode does not strip header information.
                    // Close the current scope block.
                    }
                    // Call consume_each with arguments (nitem_to_process). (inline comment notes: Consume the processed items from the input stream.)
                    consume_each(nitem_to_process);  // Consume the processed items from the input stream.
                    // Return nitem_to_process to the caller. (inline comment notes: Inform the scheduler about the number of output items produced.)
                    return nitem_to_process;  // Inform the scheduler about the number of output items produced.
                // Close the current scope block.
                }

                // Branch when condition (nitem_to_process == 0) evaluates to true. (inline comment notes: If we still do not have enough data, exit early to wait for more samples.)
                if (nitem_to_process == 0)  // If we still do not have enough data, exit early to wait for more samples.
                // Open a new scope block.
                {
                    // Return 0 to the caller. (inline comment notes: Produce no output and request more input.)
                    return 0;  // Produce no output and request more input.
                // Close the current scope block.
                }

                //we assume entire header (5 bytes) is present now. otherwise we would exit before

                // Assign uint16_t payload_len to ((in[0] << 4) + in[1]). (inline comment notes: Reconstruct the payload length from the high and low nibbles.)
                uint16_t payload_len = ((in[0] << 4) + in[1]);  // Reconstruct the payload length from the high and low nibbles.
                // Assign int cr to (in[2] & 0b1110) >> 1. (inline comment notes: Extract the coding rate bits from the third header byte.)
                int cr = (in[2] & 0b1110) >> 1;  // Extract the coding rate bits from the third header byte.
                // Assign bool has_crc to (in[2] & 0b0001). (inline comment notes: Extract the CRC present flag from the lowest bit.)
                bool has_crc = (in[2] & 0b0001);  // Extract the CRC present flag from the lowest bit.

                // Assign int head_crc to in[3]. (inline comment notes: Grab the parity nibble stored in the fourth header byte.)
                int head_crc = in[3];  // Grab the parity nibble stored in the fourth header byte.
                // Assign int checksum to (in[4] << 3) + 0b0011. (inline comment notes: Combine the checksum nibble with constant bits as defined by the LoRa spec.)
                int checksum = (in[4] << 3) + 0b0011;  // Combine the checksum nibble with constant bits as defined by the LoRa spec.

                // Assign int c4 to (in[0] & 0b1000) >> 3 ^ (in[0] & 0b0100) >> 2 ^ (in[0] & 0b0010) >> 1 ^ (in[0] & 0b0001). (inline comment notes: Recompute the first parity bit using the same relations as encoding.)
                int c4 = (in[0] & 0b1000) >> 3 ^ (in[0] & 0b0100) >> 2 ^ (in[0] & 0b0010) >> 1 ^ (in[0] & 0b0001);  // Recompute the first parity bit using the same relations as encoding.
                // Assign int c3 to (in[0] & 0b1000) >> 3 ^ (in[1] & 0b1000) >> 3 ^ (in[1] & 0b0100) >> 2 ^ (in[1] & 0b0010) >> 1 ^ (in[2] & 0b0001). (inline comment notes: Recompute the second parity bit to validate the header.)
                int c3 = (in[0] & 0b1000) >> 3 ^ (in[1] & 0b1000) >> 3 ^ (in[1] & 0b0100) >> 2 ^ (in[1] & 0b0010) >> 1 ^ (in[2] & 0b0001);  // Recompute the second parity bit to validate the header.
                // Assign int c2 to (in[0] & 0b0100) >> 2 ^ (in[1] & 0b1000) >> 3 ^ (in[1] & 0b0001) ^ (in[2] & 0b1000) >> 3 ^ (in[2] & 0b0010) >> 1. (inline comment notes: Recompute the third parity bit for error detection.)
                int c2 = (in[0] & 0b0100) >> 2 ^ (in[1] & 0b1000) >> 3 ^ (in[1] & 0b0001) ^ (in[2] & 0b1000) >> 3 ^ (in[2] & 0b0010) >> 1;  // Recompute the third parity bit for error detection.
                // Assign int c1 to (in[0] & 0b0010) >> 1 ^ (in[1] & 0b0100) >> 2 ^ (in[1] & 0b0001) ^ (in[2] & 0b0100) >> 2 ^ (in[2] & 0b0010) >> 1 ^ (in[2] & 0b0001). (inline comment notes: Recompute the fourth parity bit for the header checksum nibble.)
                int c1 = (in[0] & 0b0010) >> 1 ^ (in[1] & 0b0100) >> 2 ^ (in[1] & 0b0001) ^ (in[2] & 0b0100) >> 2 ^ (in[2] & 0b0010) >> 1 ^ (in[2] & 0b0001);  // Recompute the fourth parity bit for the header checksum nibble.
                // Assign int c0 to (in[0] & 0b0001) ^ (in[1] & 0b0010) >> 1 ^ (in[2] & 0b1000) >> 3 ^ (in[2] & 0b0100) >> 2 ^ (in[2] & 0b0010) >> 1 ^ (in[2] & 0b0001). (inline comment notes: Recompute the final parity bit.)
                int c0 = (in[0] & 0b0001) ^ (in[1] & 0b0010) >> 1 ^ (in[2] & 0b1000) >> 3 ^ (in[2] & 0b0100) >> 2 ^ (in[2] & 0b0010) >> 1 ^ (in[2] & 0b0001);  // Recompute the final parity bit.

                // Assign int parity_diff to ((c3 << 3) | (c2 << 2) | (c1 << 1) | c0) ^ (checksum & 0x0F). (inline comment notes: Compare computed parity bits against the received checksum nibble.)
                int parity_diff = ((c3 << 3) | (c2 << 2) | (c1 << 1) | c0) ^ (checksum & 0x0F);  // Compare computed parity bits against the received checksum nibble.
                // Assign int header_crc to (c4 << 1) | head_crc. (inline comment notes: Combine computed MSB parity with transmitted parity to evaluate CRC condition.)
                int header_crc = (c4 << 1) | head_crc;  // Combine computed MSB parity with transmitted parity to evaluate CRC condition.

                //only send downstream if both the header parity and the header cyclic redundancy check matched
                // Branch when condition (!parity_diff && (header_crc == 2)) evaluates to true. (inline comment notes: Accept the header when both parity bits and CRC bits match specification.)
                if (!parity_diff && (header_crc == 2))  // Accept the header when both parity bits and CRC bits match specification.
                // Open a new scope block.
                {
                    // Call publish_frame_info with arguments (cr, payload_len, has_crc, checksum & 0x01, 0). (inline comment notes: Publish the decoded header fields with no error.)
                    publish_frame_info(cr, payload_len, has_crc, checksum & 0x01, 0);  // Publish the decoded header fields with no error.
                    // Branch when condition (m_print_header) evaluates to true. (inline comment notes: Optionally print header details for debugging when enabled.)
                    if (m_print_header)  // Optionally print header details for debugging when enabled.
                    // Open a new scope block.
                    {
                        // Call printf with arguments ("Len %u CR %d CRC %d LDRO %d\n", payload_len, cr, has_crc, checksum & 0x01). (inline comment notes: Write header parameters to stdout for inspection.)
                        printf("Len %u CR %d CRC %d LDRO %d\n", payload_len, cr, has_crc, checksum & 0x01);  // Write header parameters to stdout for inspection.
                    // Close the current scope block.
                    }

                    // Branch when condition (has_crc) evaluates to true. (inline comment notes: If the payload carries a CRC, set the per-frame CRC status accordingly.)
                    if (has_crc)  // If the payload carries a CRC, set the per-frame CRC status accordingly.
                    // Open a new scope block.
                    {
                        // Assign m_has_crc to true. (inline comment notes: Indicate that a CRC section should be expected at the end of the payload.)
                        m_has_crc = true;  // Indicate that a CRC section should be expected at the end of the payload.
                        // Assign m_crc_ok to false. (inline comment notes: Reset the CRC validity flag until computed later.)
                        m_crc_ok = false;  // Reset the CRC validity flag until computed later.
                    // Close the current scope block.
                    }
                    // Handle the alternative branch when previous conditions fail. (inline comment notes: Otherwise no CRC bytes are present.)
                    else  // Otherwise no CRC bytes are present.
                    // Open a new scope block.
                    {
                        // Assign m_has_crc to false. (inline comment notes: Clear the CRC flag so downstream knows only payload bytes follow.)
                        m_has_crc = false;  // Clear the CRC flag so downstream knows only payload bytes follow.
                        // Assign m_crc_ok to false. (inline comment notes: Reset the CRC flag even though it will remain unused.)
                        m_crc_ok = false;  // Reset the CRC flag even though it will remain unused.
                    // Close the current scope block.
                    }

                    // Assign is_header to false. (inline comment notes: Mark that header processing has completed so subsequent bytes are payload.)
                    is_header = false;  // Mark that header processing has completed so subsequent bytes are payload.
                // Close the current scope block.
                }
                // Handle the alternative branch when previous conditions fail. (inline comment notes: Header parity check failed, meaning the header is invalid or corrupted.)
                else  // Header parity check failed, meaning the header is invalid or corrupted.
                // Open a new scope block.
                {
                    // Call publish_frame_info with arguments (cr, payload_len, has_crc, checksum & 0x01, 1). (inline comment notes: Publish the decoded values but flag an error so downstream can react.)
                    publish_frame_info(cr, payload_len, has_crc, checksum & 0x01, 1);  // Publish the decoded values but flag an error so downstream can react.
                    // Branch when condition (m_print_header) evaluates to true. (inline comment notes: If printing is enabled, notify about the failure.)
                    if (m_print_header)  // If printing is enabled, notify about the failure.
                    // Open a new scope block.
                    {
                        // Call printf with arguments ("Header decoding error\n"). (inline comment notes: Print a diagnostic message indicating header corruption.)
                        printf("Header decoding error\n");  // Print a diagnostic message indicating header corruption.
                    // Close the current scope block.
                    }
                    // Assign is_header to false. (inline comment notes: Drop header mode to avoid repeatedly reprocessing the same corrupted bytes.)
                    is_header = false;  // Drop header mode to avoid repeatedly reprocessing the same corrupted bytes.
                // Close the current scope block.
                }

                // Call consume_each with arguments (5). (inline comment notes: Consume the five header bytes that were read from the input stream.)
                consume_each(5);  // Consume the five header bytes that were read from the input stream.
                // Return 0 to the caller. (inline comment notes: Produce no output because header bytes are not forwarded downstream.)
                return 0;  // Produce no output because header bytes are not forwarded downstream.
            // Close the current scope block.
            }

            // Iterate with loop parameters (int i = 0; i < nitem_to_process; i++). (inline comment notes: Forward payload or CRC bytes when not in header mode.)
            for (int i = 0; i < nitem_to_process; i++)  // Forward payload or CRC bytes when not in header mode.
            // Open a new scope block.
            {
                // Assign out[i] to in[i]. (inline comment notes: Copy the input byte directly to the output buffer.)
                out[i] = in[i];  // Copy the input byte directly to the output buffer.
            // Close the current scope block.
            }

            // Call consume_each with arguments (nitem_to_process). (inline comment notes: Inform the scheduler how many input bytes were consumed in this pass.)
            consume_each(nitem_to_process);  // Inform the scheduler how many input bytes were consumed in this pass.
            // Return nitem_to_process to the caller. (inline comment notes: Report the number of output bytes generated.)
            return nitem_to_process;  // Report the number of output bytes generated.
        // Close the current scope block.
        }

    // Close the current scope and emit the trailing comment. (inline comment notes: Close the LoRa SDR namespace scope.)
    } /* namespace lora_sdr */  // Close the LoRa SDR namespace scope.
// Close the current scope and emit the trailing comment. (inline comment notes: Close the GNU Radio namespace scope.)
} /* namespace gr */  // Close the GNU Radio namespace scope.
