// Only compile the following section when HAVE_CONFIG_H is defined.
#ifdef HAVE_CONFIG_H
// Bring in declarations from "config.h".
#include "config.h"
// Close the preceding conditional compilation block.
#endif

// Bring in declarations from <gnuradio/io_signature.h>.
#include <gnuradio/io_signature.h>
// Bring in declarations from "dewhitening_impl.h".
#include "dewhitening_impl.h"
// Bring in declarations from "tables.h".
#include "tables.h"
// Bring in declarations from <gnuradio/lora_sdr/utilities.h>.
#include <gnuradio/lora_sdr/utilities.h>

// Start namespace gr scope.
namespace gr
// Open a new scope block.
{
    // Start namespace lora_sdr scope.
    namespace lora_sdr
    // Open a new scope block.
    {

        // Specify the shared pointer typedef associated with dewhitening.
        dewhitening::sptr
        // Define the g::make function.
        dewhitening::make()
        // Open a new scope block.
        {
            // Return gnuradio::get_initial_sptr(new dewhitening_impl()) to the caller.
            return gnuradio::get_initial_sptr(new dewhitening_impl());
        // Close the current scope block.
        }

        /*
     * The private constructor
     */
        // Define the l::dewhitening_impl function.
        dewhitening_impl::dewhitening_impl()
            // Initialize base classes or members with gr::block("dewhitening",.
            : gr::block("dewhitening",
                        // Specify parameter or initializer gr::io_signature::make(1, 1, sizeof(uint8_t)).
                        gr::io_signature::make(1, 1, sizeof(uint8_t)),
                        // Define the e::make function.
                        gr::io_signature::make(1, 1, sizeof(uint8_t)))
        // Open a new scope block.
        {
            // Call set_tag_propagation_policy with arguments (TPP_DONT).
            set_tag_propagation_policy(TPP_DONT);
        // Close the current scope block.
        }

        /*
     * Our virtual destructor.
     */
        // Define the l::~dewhitening_impl function.
        dewhitening_impl::~dewhitening_impl()
        // Open a new scope block.
        {
        // Close the current scope block.
        }

        // Define the l::forecast function.
        void dewhitening_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required)
        // Open a new scope block.
        {
            // Assign ninput_items_required[0] to 2.
            ninput_items_required[0] = 2;
        // Close the current scope block.
        }

        // Define the l::header_pay_len_handler function.
        void dewhitening_impl::header_pay_len_handler(pmt::pmt_t payload_len)
        // Open a new scope block.
        {
            // Assign m_payload_len to pmt::to_long(payload_len).
            m_payload_len = pmt::to_long(payload_len);
        // Close the current scope and emit the trailing comment.
        };

        // Define the l::new_frame_handler function.
        void dewhitening_impl::new_frame_handler(pmt::pmt_t id)
        // Open a new scope block.
        {
            // Assign offset to 0.
            offset = 0;
        // Close the current scope block.
        }
        // Define the l::header_crc_handler function.
        void dewhitening_impl::header_crc_handler(pmt::pmt_t crc_presence)
        // Open a new scope block.
        {
            // Assign m_crc_presence to pmt::to_long(crc_presence).
            m_crc_presence = pmt::to_long(crc_presence);
        // Close the current scope and emit the trailing comment.
        };

        // Specify parameter or initializer int dewhitening_impl::general_work(int noutput_items.
        int dewhitening_impl::general_work(int noutput_items,
                                           // Specify parameter or initializer gr_vector_int &ninput_items.
                                           gr_vector_int &ninput_items,
                                           // Specify parameter or initializer gr_vector_const_void_star &input_items.
                                           gr_vector_const_void_star &input_items,
                                           // Execute gr_vector_void_star &output_items).
                                           gr_vector_void_star &output_items)
        // Open a new scope block.
        {
            // Assign const uint8_t *in to (const uint8_t *)input_items[0].
            const uint8_t *in = (const uint8_t *)input_items[0];
            // Assign uint8_t *out to (uint8_t *)output_items[0].
            uint8_t *out = (uint8_t *)output_items[0];
            // Assign int nitem_to_process to ninput_items[0].
            int nitem_to_process = ninput_items[0];

            // Declare uint8_t low_nib, high_nib.
            uint8_t low_nib, high_nib;

            // Declare std::vector<tag_t> tags.
            std::vector<tag_t> tags;
            // Call get_tags_in_window with arguments (tags, 0, 0, ninput_items[0], pmt::string_to_symbol("frame_info")).
            get_tags_in_window(tags, 0, 0, ninput_items[0], pmt::string_to_symbol("frame_info"));
            // Branch when condition (tags.size()) evaluates to true.
            if (tags.size())
            // Open a new scope block.
            {
                // Branch when condition (tags[0].offset != nitems_read(0)) evaluates to true.
                if (tags[0].offset != nitems_read(0))           
                    // Assign nitem_to_process to tags[0].offset - nitems_read(0).
                    nitem_to_process = tags[0].offset - nitems_read(0);
                    
                // Handle the alternative branch when previous conditions fail.
                else
                // Open a new scope block.
                {
                    // Branch when condition (tags.size() >= 2) evaluates to true.
                    if (tags.size() >= 2)
                        // Assign nitem_to_process to tags[1].offset - tags[0].offset.
                        nitem_to_process = tags[1].offset - tags[0].offset;
                    
                    // Assign pmt::pmt_t err to pmt::string_to_symbol("error").
                    pmt::pmt_t err = pmt::string_to_symbol("error");
                    // Assign m_crc_presence to pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("crc"), err)).
                    m_crc_presence = pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("crc"), err));
                    // Assign m_payload_len to pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("pay_len"), err)).
                    m_payload_len = pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("pay_len"), err));
                    // Assign offset to 0.
                    offset = 0;
                    // Assign tags[0].offset to nitems_written(0).
                    tags[0].offset = nitems_written(0);
                    // Call add_item_tag with arguments (0, tags[0]).
                    add_item_tag(0, tags[0]);
                    // std::cout<<"\ndewi_crc "<<tags[0].offset<<" - crc: "<<(int)m_crc_presence<<" - pay_len: "<<(int)m_payload_len<<"\n";
                // Close the current scope block.
                }
            // Close the current scope block.
            }

            // Iterate with loop parameters (int i = 0; i < nitem_to_process / 2; i++).
            for (int i = 0; i < nitem_to_process / 2; i++)
            // Open a new scope block.
            {

                // Branch when condition (offset < m_payload_len) evaluates to true.
                if (offset < m_payload_len)
                // Open a new scope block.
                {
                    // Assign low_nib to in[2 * i] ^ (whitening_seq[offset] & 0x0F).
                    low_nib = in[2 * i] ^ (whitening_seq[offset] & 0x0F);
                    // Assign high_nib to in[2 * i + 1] ^ (whitening_seq[offset] & 0xF0) >> 4.
                    high_nib = in[2 * i + 1] ^ (whitening_seq[offset] & 0xF0) >> 4;
                    // Append high_nib << 4 | low_nib to dewhitened.
                    dewhitened.push_back(high_nib << 4 | low_nib);
                // Close the current scope block.
                }
                // Define the f function.
                else if ((offset < m_payload_len + 2) && m_crc_presence)
                // Open a new scope block. (inline comment notes: do not dewhiten the CRC)
                { //do not dewhiten the CRC
                    // Assign low_nib to in[2 * i].
                    low_nib = in[2 * i];
                    // Assign high_nib to in[2 * i + 1].
                    high_nib = in[2 * i + 1];
                    // Append high_nib << 4 | low_nib to dewhitened.
                    dewhitened.push_back(high_nib << 4 | low_nib);
                // Close the current scope block.
                }
                // Handle the alternative branch when previous conditions fail.
                else
                // Open a new scope block. (inline comment notes: full packet received)
                { // full packet received
                    // Exit the nearest enclosing loop or switch.
                    break;
                // Close the current scope block.
                }
                // Increment offset.
                offset++;
            // Close the current scope block.
            }
// Only compile the following section when GRLORA_DEBUG is defined.
#ifdef GRLORA_DEBUG
            // Iterate with loop parameters (unsigned int i = 0; i < dewhitened.size(); i++).
            for (unsigned int i = 0; i < dewhitened.size(); i++)
            // Open a new scope block.
            {
                // Execute statement std::cout << (char)(int)dewhitened[i] << "    0x" << std::hex << (int)dewhitened[i] << std::dec << std::endl.
                std::cout << (char)(int)dewhitened[i] << "    0x" << std::hex << (int)dewhitened[i] << std::dec << std::endl;
            // Close the current scope block.
            }
// Close the preceding conditional compilation block.
#endif
            // Call consume_each with arguments (dewhitened.size() * 2). (inline comment notes: ninput_items[0]/2*2);)
            consume_each(dewhitened.size() * 2); //ninput_items[0]/2*2);
            // Assign noutput_items to dewhitened.size().
            noutput_items = dewhitened.size();
            // Call memcpy with arguments (out, &dewhitened[0], noutput_items * sizeof(uint8_t)).
            memcpy(out, &dewhitened[0], noutput_items * sizeof(uint8_t));

            // Clear all contents from dewhitened.
            dewhitened.clear();
            // Return noutput_items to the caller.
            return noutput_items;
        // Close the current scope block.
        }
    // Close the current scope and emit the trailing comment.
    } /* namespace lora */
// Close the current scope and emit the trailing comment.
} /* namespace gr */
