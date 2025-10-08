// Check whether the build system provided a configuration header.
#ifdef HAVE_CONFIG_H // Begin conditional inclusion of config.h when available.
// Pull in project configuration macros generated at configure time.
#include "config.h" // Include configuration definitions.
#endif // End HAVE_CONFIG_H conditional block.

// Bring in GNU Radio block signature utilities.
#include <gnuradio/io_signature.h> // Include helper for defining block input/output signatures.
// Include the declaration of the add_crc implementation class.
#include "add_crc_impl.h" // Include the corresponding header for this implementation.

// Begin the top-level GNU Radio namespace.
namespace gr // Enter the GNU Radio namespace.
{
    // Enter the nested namespace for the LoRa SDR components.
    namespace lora_sdr // Begin namespace containing LoRa SDR blocks.
    {

        // Declare the factory method that returns a shared pointer to the block.
        add_crc::sptr // Specify the shared pointer type defined in the base class.
        // Define the factory method that constructs the concrete implementation.
        add_crc::make(bool has_crc) // Provide the optional CRC enable flag to the constructor.
        {
            // Create and return a shared pointer to a new add_crc_impl instance.
            return gnuradio::get_initial_sptr(new add_crc_impl(has_crc)); // Allocate the block and wrap it in a smart pointer.
        }

        /*
     * The private constructor
     */
        // Define the constructor that configures the block.
        add_crc_impl::add_crc_impl(bool has_crc) // Store whether CRC must be appended.
            : gr::block("add_crc", // Call the base class constructor with the block name.
                        gr::io_signature::make(1, 1, sizeof(uint8_t)), // Declare a single input stream of bytes.
                        gr::io_signature::make(1, 1, sizeof(uint8_t))) // Declare a single output stream of bytes.
        {
            // Cache the CRC enable flag for later use.
            m_has_crc = has_crc; // Record whether the block should append a CRC to payloads.
            // Prevent GNU Radio from copying tags from input to output automatically.
            set_tag_propagation_policy(TPP_DONT); // Disable automatic tag propagation since tags are forwarded manually.
        }

        /*
     * Our virtual destructor.
     */
        // Provide a virtual destructor even though no special cleanup is required.
        add_crc_impl::~add_crc_impl() // Destructor definition for completeness.
        {
        }

        // Inform the scheduler how many input items are needed to produce output.
        void // Return type of the forecast method is void.
        add_crc_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required) // Provide scheduler with required input items.
        {
            // Require at least one input item for every requested output item chunk.
            ninput_items_required[0] = 1; // Ensure the scheduler supplies data continuously.
        }

        // Compute the CRC-16/CCITT value iteratively.
        unsigned int add_crc_impl::crc16(unsigned int crcValue, unsigned char newByte) // Update CRC value using the given byte.
        {
            // Declare a loop counter that iterates over each bit of the new byte.
            unsigned char i; // Loop index for bitwise processing.
            for (i = 0; i < 8; i++) // Process the eight bits of the incoming byte.
            {

                // Test whether the current MSB of the CRC differs from the MSB of the byte.
                if (((crcValue & 0x8000) >> 8) ^ (newByte & 0x80)) // Check if a polynomial XOR is required.
                {
                    // Shift left and XOR with the CRC polynomial when bits differ.
                    crcValue = (crcValue << 1) ^ 0x1021; // Apply the CCITT polynomial.
                }
                else
                {
                    // Otherwise only shift the CRC accumulator left by one bit.
                    crcValue = (crcValue << 1); // Shift without polynomial XOR when MSBs match.
                }
                // Move to the next bit of the input byte.
                newByte <<= 1; // Prepare the next bit for processing.
            }
            // Return the updated CRC value after processing all bits.
            return crcValue; // Provide the new CRC accumulator back to the caller.
        }

        // void msg_handler(pmt::pmt_t message)
        // {
        //     std::string str = pmt::symbol_to_string(message);
        //     std::copy(str.begin(), str.end(), std::back_inserter(m_payload));
        // }

        // Main work function invoked by the GNU Radio scheduler.
        int add_crc_impl::general_work(int noutput_items, // Maximum number of output items requested by scheduler.
                                       gr_vector_int &ninput_items, // Number of available items on each input.
                                       gr_vector_const_void_star &input_items, // Pointers to input buffers.
                                       gr_vector_void_star &output_items) // Pointers to output buffers.
        {
            // Interpret the first input stream as bytes.
            const uint8_t *in = (const uint8_t *)input_items[0]; // Cast the incoming buffer to byte pointers.
            // Interpret the first output stream as writable bytes.
            uint8_t *out = (uint8_t *)output_items[0]; // Cast the output buffer to byte pointers.
            // Track how many items will be produced this call.
            int nitems_to_output = 0; // Initialize output item counter.
            // Leave room for a potential CRC when reporting availability.
            noutput_items = std::max(0, noutput_items - 4); // Reserve four slots for CRC nibble output.
            // Determine how many input items to process based on availability.
            int nitems_to_process = std::min(ninput_items[0], noutput_items); // Process only what is available and permitted.

            // read tags
            // Collect relevant tags within the current window.
            std::vector<tag_t> tags; // Container for located tags.
            get_tags_in_window(tags, 0, 0, noutput_items, pmt::string_to_symbol("payload_str")); // Search for payload tags on the input stream.
            if (tags.size()) // Continue processing when a payload tag is found.
            {

                // Check whether the first tag aligns with the current read index.
                if (tags[0].offset != nitems_read(0)) // Verify tag synchronization with data stream position.
                {
                    // Limit processing to data before the next tag to avoid consuming entire frame.
                    nitems_to_process = std::min(tags[0].offset - nitems_read(0), (uint64_t)noutput_items); // Trim processing window up to the next tag.
                }
                else
                {
                    // If a subsequent tag exists, cap the processing length accordingly.
                    if (tags.size() >= 2) // Ensure there is a second tag before accessing it.
                    {
                        // Use the distance to the second tag to define processing limit.
                        nitems_to_process = std::min(tags[1].offset - tags[0].offset, (uint64_t)noutput_items); // Prevent crossing into the next tagged payload.
                    }
                    // Convert the tag payload into a string payload for CRC calculations.
                    std::string str = pmt::symbol_to_string(tags[0].value); // Extract the payload string from the tag value.
                    std::copy(str.begin(), str.end(), std::back_inserter(m_payload)); // Append payload data to the internal buffer.
                    //pass tags downstream
                    // Fetch the frame length tag to forward it after adding CRC.
                    get_tags_in_window(tags, 0, 0, ninput_items[0], pmt::string_to_symbol("frame_len")); // Retrieve frame length metadata from the input stream.
                    m_frame_len = pmt::to_long(tags[0].value); // Convert frame length to an integer.
                    tags[0].offset = nitems_written(0); // Update the tag offset to match the output position.
                    tags[0].value = pmt::from_long(m_frame_len + (m_has_crc ? 4 : 0)); // Adjust the frame length to account for CRC nibble output when enabled.
                    if (nitems_to_process) // Only add the tag when we have data to emit.
                        add_item_tag(0, tags[0]); // Forward the updated frame length tag downstream.

                    // Forward configuration tags along with the payload.
                    get_tags_in_window(tags, 0, 0, ninput_items[0], pmt::string_to_symbol("configuration")); // Locate configuration metadata tags.
                    if(tags.size() > 0) // Ensure at least one configuration tag exists before accessing it.
                    {
                        // Align the configuration tag offset with the output stream position.
                        tags[0].offset = nitems_written(0); // Update configuration tag offset to match output items.

                        if (nitems_to_process) add_item_tag(0, tags[0]); // Forward the configuration tag when emitting data.
                    }

                    // Reset the frame byte counter to start counting the new frame.
                    m_cnt = 0; // Begin counting items processed for the current frame.
                }
            }
            if (!nitems_to_process) // Bail out if there is no data to process this call.
            {
                // Return zero to indicate no output was produced.
                return 0; // Defer work until more data is available.
            }
            // Keep track of the total number of processed items for the current frame.
            m_cnt += nitems_to_process; // Update the running count of bytes handled.
            if (m_has_crc && m_cnt == m_frame_len && nitems_to_process) // Check whether it is time to append the CRC.
            { //append the CRC to the payload
                // Initialize the CRC accumulator for the payload processing.
                uint16_t crc = 0x0000; // Start CRC value at zero per CCITT specification.
                m_payload_len = m_payload.size(); // Determine how many bytes compose the payload.
                //calculate CRC on the N-2 firsts data bytes using Poly=1021 Init=0000
                for (int i = 0; i < (int)m_payload_len - 2; i++) // Iterate over all bytes except the final two CRC bytes provided in payload.
                    crc = crc16(crc, m_payload[i]); // Update the CRC with each payload byte.

                //XOR the obtained CRC with the last 2 data bytes
                crc = crc ^ m_payload[m_payload_len - 1] ^ (m_payload[m_payload_len - 2] << 8); // Combine computed CRC with provided CRC bytes to finalize value.
                //Place the CRC in the correct output nibble
                out[nitems_to_process] = ((crc & 0x000F)); // Output the least significant nibble of the CRC.
                out[nitems_to_process + 1] = ((crc & 0x00F0) >> 4); // Output the second nibble of the CRC.
                out[nitems_to_process + 2] = ((crc & 0x0F00) >> 8); // Output the third nibble of the CRC.
                out[nitems_to_process + 3] = ((crc & 0xF000) >> 12); // Output the most significant nibble of the CRC.

                // Account for the extra CRC nibbles added to the output.
                nitems_to_output = nitems_to_process + 4; // Report payload plus CRC length.
                m_payload.clear(); // Clear the payload buffer after processing the frame.
            }
            else
            {
                // When not appending CRC, produce the same number of items as consumed.
                nitems_to_output = nitems_to_process; // Output only the processed payload bytes.
            }
            // Copy the processed payload data to the output buffer.
            memcpy(out, in, nitems_to_process * sizeof(uint8_t)); // Forward the payload bytes unchanged.
            // Tell the runtime how many items were consumed from the input.
            consume_each(nitems_to_process); // Inform scheduler about consumed input items.


            // Return the number of output items produced during this invocation.
            return nitems_to_output; // Report how many bytes were written to the output buffer.

        }
    } /* namespace lora */ // Close the lora_sdr namespace comment.
} /* namespace gr */ // Close the GNU Radio namespace comment.
