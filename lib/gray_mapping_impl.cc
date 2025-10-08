// Only compile the following section when HAVE_CONFIG_H is defined.
#ifdef HAVE_CONFIG_H
// Bring in declarations from "config.h".
#include "config.h"
// Close the preceding conditional compilation block.
#endif

// Bring in declarations from <gnuradio/io_signature.h>.
#include <gnuradio/io_signature.h>
// Bring in declarations from <gnuradio/lora_sdr/utilities.h>.
#include <gnuradio/lora_sdr/utilities.h>

// Bring in declarations from "gray_mapping_impl.h".
#include "gray_mapping_impl.h"

// Start namespace gr scope.
namespace gr {
    // Start namespace lora_sdr scope.
    namespace lora_sdr {

        // Specify the shared pointer typedef associated with gray_mapping.
        gray_mapping::sptr
        // Define the g::make function.
        gray_mapping::make(bool soft_decoding) {
            // Return gnuradio::get_initial_sptr(new gray_mapping_impl( soft_decoding)) to the caller.
            return gnuradio::get_initial_sptr(new gray_mapping_impl( soft_decoding));
        // Close the current scope block.
        }

        /*
         * The private constructor
         */
        // Define the l::gray_mapping_impl function.
        gray_mapping_impl::gray_mapping_impl(bool soft_decoding)
            // Initialize base classes or members with gr::sync_block("gray_mapping",.
            : gr::sync_block("gray_mapping",
                             // Specify parameter or initializer gr::io_signature::make(1, 1, soft_decoding ? MAX_SF * sizeof(LLR) : sizeof(uint16_t)).
                             gr::io_signature::make(1, 1, soft_decoding ? MAX_SF * sizeof(LLR) : sizeof(uint16_t)),
                             // Specify parameter or initializer gr::io_signature::make(1, 1, soft_decoding ? MAX_SF * sizeof(LLR) : sizeof(uint16_t))).
                             gr::io_signature::make(1, 1, soft_decoding ? MAX_SF * sizeof(LLR) : sizeof(uint16_t))),
              // Define the g function.
              m_soft_decoding(soft_decoding) {
            // Call set_tag_propagation_policy with arguments (TPP_ONE_TO_ONE).
            set_tag_propagation_policy(TPP_ONE_TO_ONE);
        // Close the current scope block.
        }

        /*
         * Our virtual destructor.
         */
        // Define the l::~gray_mapping_impl function.
        gray_mapping_impl::~gray_mapping_impl() {}

        // Specify parameter or initializer int gray_mapping_impl::work(int noutput_items.
        int gray_mapping_impl::work(int noutput_items,
                                // Specify parameter or initializer gr_vector_const_void_star &input_items.
                                gr_vector_const_void_star &input_items,
                                // Execute gr_vector_void_star &output_items) {.
                                gr_vector_void_star &output_items) {
            // Assign const uint16_t *in1 to (const uint16_t *)input_items[0].
            const uint16_t *in1 = (const uint16_t *)input_items[0];
            // Assign const LLR *in2 to (const LLR *)input_items[0].
            const LLR *in2 = (const LLR *)input_items[0];
            // Assign uint16_t *out1 to (uint16_t *)output_items[0].
            uint16_t *out1 = (uint16_t *)output_items[0];
            // Assign LLR *out2 to (LLR *)output_items[0].
            LLR *out2 = (LLR *)output_items[0];

            // Declare std::vector<tag_t> tags.
            std::vector<tag_t> tags;
            // Assign int nitems_to_process to noutput_items.
            int nitems_to_process = noutput_items;
            // Call get_tags_in_window with arguments (tags, 0, 0, noutput_items, pmt::string_to_symbol("frame_info")).
            get_tags_in_window(tags, 0, 0, noutput_items, pmt::string_to_symbol("frame_info"));
            // Branch when condition (tags.size()) evaluates to true.
            if (tags.size())
            // Open a new scope block.
            {
                // Branch when condition (tags[0].offset != nitems_read(0)) evaluates to true.
                if (tags[0].offset != nitems_read(0))
                    // Assign nitems_to_process to tags[0].offset - nitems_read(0). (inline comment notes: only use symbol until the next frame begin (SF might change))
                    nitems_to_process = tags[0].offset - nitems_read(0); // only use symbol until the next frame begin (SF might change)

                // Handle the alternative branch when previous conditions fail.
                else
                // Open a new scope block.
                {
                    // Branch when condition (tags.size() >= 2) evaluates to true.
                    if (tags.size() >= 2)
                        // Assign nitems_to_process to tags[1].offset - tags[0].offset.
                        nitems_to_process = tags[1].offset - tags[0].offset;

                    // Assign pmt::pmt_t err to pmt::string_to_symbol("error").
                    pmt::pmt_t err = pmt::string_to_symbol("error");
                    // Assign bool is_header to pmt::to_bool(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("is_header"), err)).
                    bool is_header = pmt::to_bool(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("is_header"), err));
                    // Branch when condition (is_header) evaluates to true. (inline comment notes: new frame beginning)
                    if (is_header) // new frame beginning
                    // Open a new scope block.
                    {
                        // Assign int sf to pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("sf"), err)).
                        int sf = pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("sf"), err));
                        // Assign m_sf to sf.
                        m_sf = sf;
                    // Close the current scope block.
                    }      
                // Close the current scope block.
                }
            // Close the current scope block.
            }

            // Iterate with loop parameters (int i = 0; i < nitems_to_process; i++).
            for (int i = 0; i < nitems_to_process; i++) {
                // Branch when condition (m_soft_decoding) evaluates to true.
                if (m_soft_decoding) {
                    // No gray mapping , it has as been done directly in fft_demod block => block "bypass"
                    // Call memcpy with arguments (out2 + i * MAX_SF, in2 + i * MAX_SF, MAX_SF * sizeof(LLR)).
                    memcpy(out2 + i * MAX_SF, in2 + i * MAX_SF, MAX_SF * sizeof(LLR));
                // Close the current scope and emit the trailing comment.
                } else {
                    // Assign out1[i] to (in1[i] ^ (in1[i] >> 1u)). (inline comment notes: Gray Demap)
                    out1[i] = (in1[i] ^ (in1[i] >> 1u));  // Gray Demap
                // Close the current scope block.
                }

// Only compile the following section when GRLORA_DEBUG is defined.
#ifdef GRLORA_DEBUG
                // Invoke std::cout << std::hex << "0x" << in[i] << " ---> ".
                std::cout << std::hex << "0x" << in[i] << " ---> "
                          // Declare << "0x" << out[i] << std::dec << std::endl.
                          << "0x" << out[i] << std::dec << std::endl; 
// Close the preceding conditional compilation block.
#endif
            // Close the current scope block.
            }
            // Return nitems_to_process to the caller.
            return nitems_to_process;

        // Close the current scope block.
        }
    // Close the current scope block. (inline comment notes: namespace lora_sdr)
    }  // namespace lora_sdr
// Close the current scope and emit the trailing comment.
} /* namespace gr */
