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

// Bring in declarations from "deinterleaver_impl.h".
#include "deinterleaver_impl.h"

// Start namespace gr scope.
namespace gr {
    // Start namespace lora_sdr scope.
    namespace lora_sdr {

        // Specify the shared pointer typedef associated with deinterleaver.
        deinterleaver::sptr
        // Define the r::make function.
        deinterleaver::make(bool soft_decoding) {
            // Return gnuradio::get_initial_sptr(new deinterleaver_impl( soft_decoding)) to the caller.
            return gnuradio::get_initial_sptr(new deinterleaver_impl( soft_decoding));
        // Close the current scope block.
        }

        /*
         * The private constructor
         */
        // Define the l::deinterleaver_impl function.
        deinterleaver_impl::deinterleaver_impl( bool soft_decoding)
            // Initialize base classes or members with gr::block("deinterleaver",.
            : gr::block("deinterleaver",
                        // Specify parameter or initializer gr::io_signature::make(1, 1, soft_decoding ? MAX_SF * sizeof(LLR) : sizeof(uint16_t)). (inline comment notes: In reality: sf_app               < sf)
                        gr::io_signature::make(1, 1, soft_decoding ? MAX_SF * sizeof(LLR) : sizeof(uint16_t)),  // In reality: sf_app               < sf
                        // Specify parameter or initializer gr::io_signature::make(1, 1, soft_decoding ? 8 * sizeof(LLR) : sizeof(uint8_t))). (inline comment notes: In reality: cw_len = cr_app + 4  < 8)
                        gr::io_signature::make(1, 1, soft_decoding ? 8 * sizeof(LLR) : sizeof(uint8_t))),   // In reality: cw_len = cr_app + 4  < 8
              // Define the g function.
              m_soft_decoding(soft_decoding)
               // Open a new scope block.
               {
            // Call set_tag_propagation_policy with arguments (TPP_DONT).
            set_tag_propagation_policy(TPP_DONT);
        // Close the current scope block.
        }

        /*
         * Our virtual destructor.
         */
        // Define the l::~deinterleaver_impl function.
        deinterleaver_impl::~deinterleaver_impl() {}

        // Define the l::forecast function.
        void deinterleaver_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required) {
            // Assign ninput_items_required[0] to 4.
            ninput_items_required[0] = 4;
        // Close the current scope block.
        }

        // Specify parameter or initializer int deinterleaver_impl::general_work(int noutput_items.
        int deinterleaver_impl::general_work(int noutput_items,
                                             // Specify parameter or initializer gr_vector_int &ninput_items.
                                             gr_vector_int &ninput_items,
                                             // Specify parameter or initializer gr_vector_const_void_star &input_items.
                                             gr_vector_const_void_star &input_items,
                                             // Execute gr_vector_void_star &output_items) {.
                                             gr_vector_void_star &output_items) {
            // Assign const uint16_t *in1 to (const uint16_t *)input_items[0].
            const uint16_t *in1 = (const uint16_t *)input_items[0];
            // Assign const LLR *in2 to (const LLR *)input_items[0].
            const LLR *in2 = (const LLR *)input_items[0];
            // Assign uint8_t *out1 to (uint8_t *)output_items[0].
            uint8_t *out1 = (uint8_t *)output_items[0];
            // Assign LLR *out2 to (LLR *)output_items[0].
            LLR *out2 = (LLR *)output_items[0];

            // Declare std::vector<tag_t> tags.
            std::vector<tag_t> tags;
            // Call get_tags_in_window with arguments (tags, 0, 0, 1, pmt::string_to_symbol("frame_info")).
            get_tags_in_window(tags, 0, 0, 1, pmt::string_to_symbol("frame_info"));
            // Branch when condition (tags.size()) evaluates to true.
            if (tags.size()) {
                // Assign pmt::pmt_t err to pmt::string_to_symbol("error").
                pmt::pmt_t err = pmt::string_to_symbol("error");
                // Assign m_is_header to pmt::to_bool(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("is_header"), err)).
                m_is_header = pmt::to_bool(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("is_header"), err));
                
                // Branch when condition (m_is_header) evaluates to true.
                if (m_is_header) {
                    // Assign m_sf to pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("sf"), err)).
                    m_sf = pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("sf"), err));
                    // std::cout<<"deinterleaver_header "<<tags[0].offset<<std::endl;
                    // is_first = true;
                // Close the current scope and emit the trailing comment.
                } else {
                    // is_first=false;
                    // Assign m_cr to pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("cr"), err)).
                    m_cr = pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("cr"), err));
                    // Assign m_ldro to pmt::to_bool(pmt::dict_ref(tags[0].value,pmt::string_to_symbol("ldro"),err)).
                    m_ldro = pmt::to_bool(pmt::dict_ref(tags[0].value,pmt::string_to_symbol("ldro"),err));
                    // std::cout<<"\ndeinter_cr "<<tags[0].offset<<" - cr: "<<(int)m_cr<<"\n";
                // Close the current scope block.
                }
                // Assign tags[0].offset to nitems_written(0).
                tags[0].offset = nitems_written(0);
                // Call add_item_tag with arguments (0, tags[0]).
                add_item_tag(0, tags[0]);

            // Close the current scope block.
            }
            // Assign sf_app to (m_is_header||m_ldro) ? m_sf - 2 : m_sf. (inline comment notes: Use reduced rate for the first block)
            sf_app = (m_is_header||m_ldro) ? m_sf - 2 : m_sf;  // Use reduced rate for the first block
            // Assign cw_len to m_is_header ? 8 : m_cr + 4.
            cw_len = m_is_header ? 8 : m_cr + 4;
            // std::cout << "sf_app " << +sf_app << " cw_len " << +cw_len << std::endl;

            // Branch when condition (ninput_items[0] >= cw_len) evaluates to true. (inline comment notes: wait for a full block to deinterleave)
            if (ninput_items[0] >= cw_len) {  // wait for a full block to deinterleave

                // Branch when condition (m_soft_decoding) evaluates to true.
                if (m_soft_decoding) {
                    // Create the empty matrices
                    // Execute statement std::vector<LLR> init_LLR1(sf_app, 0).
                    std::vector<LLR> init_LLR1(sf_app, 0);
                    // Execute statement std::vector<std::vector<LLR>> inter_bin(cw_len, init_LLR1).
                    std::vector<std::vector<LLR>> inter_bin(cw_len, init_LLR1);
                    // Execute statement std::vector<LLR> init_LLR2(cw_len, 0).
                    std::vector<LLR> init_LLR2(cw_len, 0);
                    // Execute statement std::vector<std::vector<LLR>> deinter_bin(sf_app, init_LLR2).
                    std::vector<std::vector<LLR>> deinter_bin(sf_app, init_LLR2);

                    // Iterate with loop parameters (uint32_t i = 0; i < cw_len; i++).
                    for (uint32_t i = 0; i < cw_len; i++) {
                        // take only sf_app bits over the sf bits available
                        // Call memcpy with arguments (inter_bin[i].data(), in2 + (i * MAX_SF + m_sf - sf_app), sf_app * sizeof(LLR)).
                        memcpy(inter_bin[i].data(), in2 + (i * MAX_SF + m_sf - sf_app), sf_app * sizeof(LLR));
                    // Close the current scope block.
                    }

                    // Do the actual deinterleaving
                    // Iterate with loop parameters (int32_t i = 0; i < cw_len; i++).
                    for (int32_t i = 0; i < cw_len; i++) {
                        // Iterate with loop parameters (int32_t j = 0; j < int(sf_app); j++).
                        for (int32_t j = 0; j < int(sf_app); j++) {
                            // std::cout << "T["<<i<<"]["<<j<<"] "<< (inter_bin[i][j] > 0) << " ";
                            // Assign deinter_bin[mod((i - j - 1), sf_app)][i] to inter_bin[i][j].
                            deinter_bin[mod((i - j - 1), sf_app)][i] = inter_bin[i][j];
                        // Close the current scope block.
                        }
                        // std::cout << std::endl;
                    // Close the current scope block.
                    }

                    // Iterate with loop parameters (uint32_t i = 0; i < sf_app; i++).
                    for (uint32_t i = 0; i < sf_app; i++) {
                        // Write only the cw_len bits over the 8 bits space available
                        // Call memcpy with arguments (out2 + i * 8, deinter_bin[i].data(), cw_len * sizeof(LLR)).
                        memcpy(out2 + i * 8, deinter_bin[i].data(), cw_len * sizeof(LLR));
                    // Close the current scope block.
                    }

                // Close the current scope block.
                } 
                // Handle the alternative branch when previous conditions fail. (inline comment notes: Hard-Decoding)
                else {  // Hard-Decoding
                    // Create the empty matrices
                    // Execute statement std::vector<std::vector<bool>> inter_bin(cw_len).
                    std::vector<std::vector<bool>> inter_bin(cw_len);
                    // Execute statement std::vector<bool> init_bit(cw_len, 0).
                    std::vector<bool> init_bit(cw_len, 0);
                    // Execute statement std::vector<std::vector<bool>> deinter_bin(sf_app, init_bit).
                    std::vector<std::vector<bool>> deinter_bin(sf_app, init_bit);

                    // convert decimal vector to binary vector of vector
                    // Iterate with loop parameters (int i = 0; i < cw_len; i++).
                    for (int i = 0; i < cw_len; i++) {
                        // Assign inter_bin[i] to int2bool(in1[i], sf_app).
                        inter_bin[i] = int2bool(in1[i], sf_app);
                    // Close the current scope block.
                    }
// Only compile the following section when GRLORA_DEBUG is defined.
#ifdef GRLORA_DEBUG
                    // Declare std::cout << "interleaved----" << std::endl.
                    std::cout << "interleaved----" << std::endl;
                    // Iterate with loop parameters (uint32_t i = 0u; i < cw_len; i++).
                    for (uint32_t i = 0u; i < cw_len; i++) {
                        // Iterate with loop parameters (int j = 0; j < int(sf_app); j++).
                        for (int j = 0; j < int(sf_app); j++) {
                            // Declare std::cout << inter_bin[i][j].
                            std::cout << inter_bin[i][j];
                        // Close the current scope block.
                        }
                        // Execute statement std::cout << " " << (int)in1[i] << std::endl.
                        std::cout << " " << (int)in1[i] << std::endl;
                    // Close the current scope block.
                    }
                    // Declare std::cout << std::endl.
                    std::cout << std::endl;
// Close the preceding conditional compilation block.
#endif
                    // Do the actual deinterleaving
                    // Iterate with loop parameters (int32_t i = 0; i < cw_len; i++).
                    for (int32_t i = 0; i < cw_len; i++) {
                        // Iterate with loop parameters (int32_t j = 0; j < int(sf_app); j++).
                        for (int32_t j = 0; j < int(sf_app); j++) {
                            // std::cout << "T["<<i<<"]["<<j<<"] "<< inter_bin[i][j] << " ";
                            // Assign deinter_bin[mod((i - j - 1), sf_app)][i] to inter_bin[i][j].
                            deinter_bin[mod((i - j - 1), sf_app)][i] = inter_bin[i][j];
                        // Close the current scope block.
                        }
                        // std::cout << std::endl;
                    // Close the current scope block.
                    }

                    // transform codewords from binary vector to dec
                    // Iterate with loop parameters (unsigned int i = 0; i < sf_app; i++).
                    for (unsigned int i = 0; i < sf_app; i++) {
                        // Assign out1[i] to bool2int(deinter_bin[i]). (inline comment notes: bool2int return uint32_t Maybe explicit conversion to uint8_t)
                        out1[i] = bool2int(deinter_bin[i]);  // bool2int return uint32_t Maybe explicit conversion to uint8_t
                    // Close the current scope block.
                    }

// Only compile the following section when GRLORA_DEBUG is defined.
#ifdef GRLORA_DEBUG
                    // Declare std::cout << "codewords----" << std::endl.
                    std::cout << "codewords----" << std::endl;
                    // Iterate with loop parameters (uint32_t i = 0u; i < sf_app; i++).
                    for (uint32_t i = 0u; i < sf_app; i++) {
                        // Iterate with loop parameters (int j = 0; j < int(cw_len); j++).
                        for (int j = 0; j < int(cw_len); j++) {
                            // Declare std::cout << deinter_bin[i][j].
                            std::cout << deinter_bin[i][j];
                        // Close the current scope block.
                        }
                        // Execute statement std::cout << " 0x" << std::hex << (int)out1[i] << std::dec << std::endl.
                        std::cout << " 0x" << std::hex << (int)out1[i] << std::dec << std::endl;
                    // Close the current scope block.
                    }
                    // Declare std::cout << std::endl.
                    std::cout << std::endl;
// Close the preceding conditional compilation block.
#endif
                    // if(is_first)
                    //     add_item_tag(0, nitems_written(0), pmt::string_to_symbol("header_len"), pmt::mp((long)sf_app));//sf_app is the header part size

                    // consume_each(cw_len);
                // Close the current scope block.
                }
                // Call consume_each with arguments (cw_len).
                consume_each(cw_len);

                // Branch when condition (noutput_items < sf_app) evaluates to true.
                if (noutput_items < sf_app)
                    // Declare std::cout << RED << "[deinterleaver.cc] Not enough output space! " << noutput_items << "/" << sf_app << std::endl.
                    std::cout << RED << "[deinterleaver.cc] Not enough output space! " << noutput_items << "/" << sf_app << std::endl;

                // Return sf_app to the caller.
                return sf_app;
            // Close the current scope block.
            }
            // Return 0 to the caller.
            return 0;
        // Close the current scope block.
        }
    // Close the current scope block. (inline comment notes: namespace lora_sdr)
    }  // namespace lora_sdr
// Close the current scope and emit the trailing comment.
} /* namespace gr */
