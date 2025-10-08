
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

// Bring in declarations from "hamming_dec_impl.h".
#include "hamming_dec_impl.h"

// Only compile the following section when GRLORA_DEBUG is defined.
#ifdef GRLORA_DEBUG
// Bring in declarations from <algorithm>. (inline comment notes: find in LUT)
#include <algorithm>  // find in LUT
// Bring in declarations from <bitset>. (inline comment notes: debug bit)
#include <bitset>     // debug bit
// Close the preceding conditional compilation block.
#endif

// Start namespace gr scope.
namespace gr {
    // Start namespace lora_sdr scope.
    namespace lora_sdr {

        // Specify the shared pointer typedef associated with hamming_dec.
        hamming_dec::sptr
        // Define the c::make function.
        hamming_dec::make(bool soft_decoding) {
            // Return gnuradio::get_initial_sptr(new hamming_dec_impl(soft_decoding)) to the caller.
            return gnuradio::get_initial_sptr(new hamming_dec_impl(soft_decoding));
        // Close the current scope block.
        }

        /*
         * The private constructor
         */
        // Define the l::hamming_dec_impl function.
        hamming_dec_impl::hamming_dec_impl(bool soft_decoding)
            // Initialize base classes or members with gr::sync_block("hamming_dec",.
            : gr::sync_block("hamming_dec",
                             // Specify parameter or initializer gr::io_signature::make(1, 1, soft_decoding ? 8 * sizeof(LLR) : sizeof(uint8_t)). (inline comment notes: In reality: cw_len = cr_app + 4  < 8)
                             gr::io_signature::make(1, 1, soft_decoding ? 8 * sizeof(LLR) : sizeof(uint8_t)),  // In reality: cw_len = cr_app + 4  < 8
                             // Specify parameter or initializer gr::io_signature::make(1, 1, sizeof(uint8_t))).
                             gr::io_signature::make(1, 1, sizeof(uint8_t))),
              // Define the g function.
              m_soft_decoding(soft_decoding) {
            // Call set_tag_propagation_policy with arguments (TPP_ONE_TO_ONE).
            set_tag_propagation_policy(TPP_ONE_TO_ONE);
        // Close the current scope block.
        }
        /*
         * Our virtual destructor.
         */
        // Define the l::~hamming_dec_impl function.
        hamming_dec_impl::~hamming_dec_impl() {
        // Close the current scope block.
        }

        // Specify parameter or initializer int hamming_dec_impl::work(int noutput_items.
        int hamming_dec_impl::work(int noutput_items,
                                   // Specify parameter or initializer gr_vector_const_void_star &input_items.
                                   gr_vector_const_void_star &input_items,
                                   // Execute gr_vector_void_star &output_items) {.
                                   gr_vector_void_star &output_items) {
            // Assign const uint8_t *in to (const uint8_t *)input_items[0].
            const uint8_t *in = (const uint8_t *)input_items[0];
            // Assign const LLR *in2 to (const LLR *)input_items[0].
            const LLR *in2 = (const LLR *)input_items[0];
            // Assign uint8_t *out to (uint8_t *)output_items[0].
            uint8_t *out = (uint8_t *)output_items[0];
            // Assign int nitems_to_process to noutput_items.
            int nitems_to_process = noutput_items;

            // Declare std::vector<tag_t> tags.
            std::vector<tag_t> tags;
            // Call get_tags_in_window with arguments (tags, 0, 0, noutput_items, pmt::string_to_symbol("frame_info")).
            get_tags_in_window(tags, 0, 0, noutput_items, pmt::string_to_symbol("frame_info"));
            // Branch when condition (tags.size()) evaluates to true.
            if (tags.size()) {
                // Branch when condition (tags[0].offset != nitems_read(0)) evaluates to true.
                if (tags[0].offset != nitems_read(0))
                    // Assign nitems_to_process to tags[0].offset - nitems_read(0). (inline comment notes: only decode codewords until the next frame begin)
                    nitems_to_process = tags[0].offset - nitems_read(0);  // only decode codewords until the next frame begin

                // Handle the alternative branch when previous conditions fail.
                else {
                    // Branch when condition (tags.size() >= 2) evaluates to true.
                    if (tags.size() >= 2)
                        // Assign nitems_to_process to tags[1].offset - tags[0].offset.
                        nitems_to_process = tags[1].offset - tags[0].offset;

                    // Assign pmt::pmt_t err to pmt::string_to_symbol("error").
                    pmt::pmt_t err = pmt::string_to_symbol("error");
                    // Assign is_header to pmt::to_bool(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("is_header"), err)).
                    is_header = pmt::to_bool(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("is_header"), err));

                    // Branch when condition (!is_header) evaluates to true.
                    if (!is_header) {
                        // Assign m_cr to pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("cr"), err)).
                        m_cr = pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("cr"), err));
                        // std::cout<<"\nhamming_cr "<<tags[0].offset<<" - cr: "<<(int)m_cr<<"\n";
                    // Close the current scope block.
                    }
                // Close the current scope block.
                }
            // Close the current scope block.
            }

            // Assign cr_app to is_header ? 4 : m_cr.
            cr_app = is_header ? 4 : m_cr;
            // Assign uint8_t cw_len to cr_app + 4.
            uint8_t cw_len = cr_app + 4;

            // Iterate with loop parameters (int i = 0; i < nitems_to_process; i++).
            for (int i = 0; i < nitems_to_process; i++) {
                // Branch when condition (m_soft_decoding) evaluates to true.
                if (m_soft_decoding) {
                    // Execute statement std::vector<LLR> codeword_LLR(cw_len, 0).
                    std::vector<LLR> codeword_LLR(cw_len, 0);

                    // Call memcpy with arguments (codeword_LLR.data(), in2 + i * 8, cw_len * sizeof(LLR)).
                    memcpy(codeword_LLR.data(), in2 + i * 8, cw_len * sizeof(LLR));

// Only compile the following section when GRLORA_DEBUG is defined.
#ifdef GRLORA_DEBUG
                    // convert LLR to binary for debug
                    // Execute statement uint8_t x(0).
                    uint8_t x(0);
                    // Iterate with loop parameters (int i(0); i < cw_len; i++) x += (codeword_LLR[i] > 0) << (7 - i).
                    for (int i(0); i < cw_len; i++) x += (codeword_LLR[i] > 0) << (7 - i);
                    // Execute statement std::bitset<8> X(x).
                    std::bitset<8> X(x);
                    // Declare std::cout << "Hamming in-symbol: " << +x << " " << X << std::endl.
                    std::cout << "Hamming in-symbol: " << +x << " " << X << std::endl;
// Close the preceding conditional compilation block.
#endif

                    /*  Hamming Look-up Table generation, parity bits formula with data [d0 d1 d2 d3]:
                     *      p0 = d0 ^ d1 ^ d2;     ^ = xor
                     *      p1 = d1 ^ d2 ^ d3;
                     *      p2 = d0 ^ d1 ^ d3;
                     *      p3 = d0 ^ d2 ^ d3;
                     *
                     *      p = d0 ^ d1 ^ d2 ^ d3;  for CR=4/5
                     * 
                     *      For LUT, store the decimal value instead of bit matrix, same LUT for CR 4/6, 4/7 and 4/8 (just crop)
                     *      e.g.    139 = [ 1 0 0 0 | 1 0 1 1 ] = [ d0 d1 d2 d3 | p0 p1 p2 p3]
                     */
                    // Declare constant const uint8_t cw_nbr. (inline comment notes: In LoRa, always "only" 16 possible codewords => compare with all and take argmax)
                    const uint8_t cw_nbr = 16;  // In LoRa, always "only" 16 possible codewords => compare with all and take argmax
                    // Assign uint8_t cw_LUT[cw_nbr] to {0, 23, 45, 58, 78, 89, 99, 116, 139, 156, 166, 177, 197, 210, 232, 255}.
                    uint8_t cw_LUT[cw_nbr] = {0, 23, 45, 58, 78, 89, 99, 116, 139, 156, 166, 177, 197, 210, 232, 255};
                    // Assign uint8_t cw_LUT_cr5[cw_nbr] to {0, 24, 40, 48, 72, 80, 96, 120, 136, 144, 160, 184, 192, 216, 232, 240}. (inline comment notes: Different for cr = 4/5)
                    uint8_t cw_LUT_cr5[cw_nbr] = {0, 24, 40, 48, 72, 80, 96, 120, 136, 144, 160, 184, 192, 216, 232, 240};  // Different for cr = 4/5

                    // Assign LLR cw_proba[cw_nbr] to {0}.
                    LLR cw_proba[cw_nbr] = {0};

                    // Iterate with loop parameters (int n = 0; n < cw_nbr; n++). (inline comment notes: for all possible codeword)
                    for (int n = 0; n < cw_nbr; n++) {      // for all possible codeword
                        // Iterate with loop parameters (int j = 0; j < cw_len; j++). (inline comment notes: for all codeword bits)
                        for (int j = 0; j < cw_len; j++) {  // for all codeword bits
                            // Select correct bit            from correct LUT          crop table (cr)    bit position mask
                            // Assign bool bit to (((cr_app != 1) ? cw_LUT[n] : cw_LUT_cr5[n]) >> (8 - cw_len)) & (1u << (cw_len - 1 - j)).
                            bool bit = (((cr_app != 1) ? cw_LUT[n] : cw_LUT_cr5[n]) >> (8 - cw_len)) & (1u << (cw_len - 1 - j));
                            // if LLR > 0 --> 1     if LLR < 0 --> 0
                            // Branch when condition ((bit and codeword_LLR[j] > 0) or (!bit and codeword_LLR[j] < 0)) evaluates to true. (inline comment notes: if correct bit 1-->1 or 0-->0)
                            if ((bit and codeword_LLR[j] > 0) or (!bit and codeword_LLR[j] < 0)) {  // if correct bit 1-->1 or 0-->0
                                // Assign cw_proba[n] + to abs(codeword_LLR[j]).
                                cw_proba[n] += abs(codeword_LLR[j]);
                            // Close the current scope and emit the trailing comment. (inline comment notes: if incorrect bit 0-->1 or 1-->0)
                            } else {                                  // if incorrect bit 0-->1 or 1-->0
                                // Assign cw_proba[n] - to abs(codeword_LLR[j]). (inline comment notes: penalty)
                                cw_proba[n] -= abs(codeword_LLR[j]);  // penalty
                            // Close the current scope block. (inline comment notes: can be optimized in 1 line: ... + ((cond)? 1 : -1) * abs(codeword_LLR[j]); but less readable)
                            }  // can be optimized in 1 line: ... + ((cond)? 1 : -1) * abs(codeword_LLR[j]); but less readable
                        // Close the current scope block.
                        }
                    // Close the current scope block.
                    }
                    // Select the codeword with the maximum probability (ML)
                    // Assign uint8_t idx_max to std::max_element(cw_proba, cw_proba + cw_nbr) - cw_proba.
                    uint8_t idx_max = std::max_element(cw_proba, cw_proba + cw_nbr) - cw_proba;
                    // convert LLR to binary => Hard decision
                    // Assign uint8_t data_nibble_soft to cw_LUT[idx_max] >> 4. (inline comment notes: Take data bits of the correct codeword (=> discard hamming code part))
                    uint8_t data_nibble_soft = cw_LUT[idx_max] >> 4;  // Take data bits of the correct codeword (=> discard hamming code part)

// Only compile the following section when GRLORA_DEBUG is defined.
#ifdef GRLORA_DEBUG
                    // for (int n = 0; n < cw_nbr; n++) std::cout << cw_proba[n] << std::endl;
                    // Execute statement std::cout << "correct cw " << unsigned(correct_cw) << " with proba " << cw_proba[idx_max] << " idxm " << unsigned(idx_max) << std::endl.
                    std::cout << "correct cw " << unsigned(correct_cw) << " with proba " << cw_proba[idx_max] << " idxm " << unsigned(idx_max) << std::endl;

                    /*if ( std::find(cw_LUT.begin(), cw_LUT.end(), x) != cw_LUT.end() )
                        std::cout << "LUT " << unsigned(x) << std::endl;
                    else
                        std::cout << "NOT in LUT " << unsigned(x) << std::endl;*/
// Close the preceding conditional compilation block.
#endif

                    // Output the most probable data nibble
                    // and reversed bit order MSB<=>LSB
                    // Assign out[i] to ((bool)(data_nibble_soft & 0b0001) << 3) + ((bool)(data_nibble_soft & 0b0010) << 2) + ((bool)(data_nibble_soft & 0b0100) << 1) + (bool)(data_nibble_soft & 0b1000).
                    out[i] = ((bool)(data_nibble_soft & 0b0001) << 3) + ((bool)(data_nibble_soft & 0b0010) << 2) + ((bool)(data_nibble_soft & 0b0100) << 1) + (bool)(data_nibble_soft & 0b1000);

                    
                // Close the current scope block.
                } 
                // Handle the alternative branch when previous conditions fail. (inline comment notes: Hard decoding)
                else {// Hard decoding
                    // Execute statement std::vector<bool> data_nibble(4, 0).
                    std::vector<bool> data_nibble(4, 0);
                    // Assign bool s0, s1, s2 to 0.
                    bool s0, s1, s2 = 0;
                    // Assign int syndrom to 0.
                    int syndrom = 0;
                    // Declare std::vector<bool> codeword.
                    std::vector<bool> codeword;

                    // Assign codeword to int2bool(in[i], cr_app + 4).
                    codeword = int2bool(in[i], cr_app + 4);
                    // Assign data_nibble to {codeword[3], codeword[2], codeword[1], codeword[0]}. (inline comment notes: reorganized msb-first)
                    data_nibble = {codeword[3], codeword[2], codeword[1], codeword[0]};  // reorganized msb-first

                    // Select behavior based on (cr_app).
                    switch (cr_app) {
                        // Handle switch label case 4.
                        case 4:
                            // Branch when condition (!(count(codeword.begin(), codeword.end(), true) % 2)) evaluates to true. (inline comment notes: Don't correct if even number of errors)
                            if (!(count(codeword.begin(), codeword.end(), true) % 2))  // Don't correct if even number of errors
                                // Exit the nearest enclosing loop or switch.
                                break;
                        // Handle switch label case 3.
                        case 3:
                            // get syndrom
                            // Assign s0 to codeword[0] ^ codeword[1] ^ codeword[2] ^ codeword[4].
                            s0 = codeword[0] ^ codeword[1] ^ codeword[2] ^ codeword[4];
                            // Assign s1 to codeword[1] ^ codeword[2] ^ codeword[3] ^ codeword[5].
                            s1 = codeword[1] ^ codeword[2] ^ codeword[3] ^ codeword[5];
                            // Assign s2 to codeword[0] ^ codeword[1] ^ codeword[3] ^ codeword[6].
                            s2 = codeword[0] ^ codeword[1] ^ codeword[3] ^ codeword[6];

                            // Assign syndrom to s0 + (s1 << 1) + (s2 << 2).
                            syndrom = s0 + (s1 << 1) + (s2 << 2);

                            // Select behavior based on (syndrom).
                            switch (syndrom) {
                                // Handle switch label case 5.
                                case 5:
                                    // Execute statement data_nibble[3].flip().
                                    data_nibble[3].flip();
                                    // Exit the nearest enclosing loop or switch.
                                    break;
                                // Handle switch label case 7.
                                case 7:
                                    // Execute statement data_nibble[2].flip().
                                    data_nibble[2].flip();
                                    // Exit the nearest enclosing loop or switch.
                                    break;
                                // Handle switch label case 3.
                                case 3:
                                    // Execute statement data_nibble[1].flip().
                                    data_nibble[1].flip();
                                    // Exit the nearest enclosing loop or switch.
                                    break;
                                // Handle switch label case 6.
                                case 6:
                                    // Execute statement data_nibble[0].flip().
                                    data_nibble[0].flip();
                                    // Exit the nearest enclosing loop or switch.
                                    break;
                                // Handle default switch label. (inline comment notes: either parity bit wrong or no error)
                                default:  // either parity bit wrong or no error
                                    // Exit the nearest enclosing loop or switch.
                                    break;
                            // Close the current scope block.
                            }
                            // Exit the nearest enclosing loop or switch.
                            break;
                        // Handle switch label case 2.
                        case 2:
                            // Assign s0 to codeword[0] ^ codeword[1] ^ codeword[2] ^ codeword[4].
                            s0 = codeword[0] ^ codeword[1] ^ codeword[2] ^ codeword[4];
                            // Assign s1 to codeword[1] ^ codeword[2] ^ codeword[3] ^ codeword[5].
                            s1 = codeword[1] ^ codeword[2] ^ codeword[3] ^ codeword[5];

                            // Branch when condition (s0 | s1) evaluates to true.
                            if (s0 | s1) {
                            // Close the current scope block.
                            }
                            // Exit the nearest enclosing loop or switch.
                            break;
                        // Handle switch label case 1.
                        case 1:
                            // Branch when condition (!(count(codeword.begin(), codeword.end(), true) % 2)) evaluates to true.
                            if (!(count(codeword.begin(), codeword.end(), true) % 2)) {
                            // Close the current scope block.
                            }
                            // Exit the nearest enclosing loop or switch.
                            break;
                    // Close the current scope block.
                    }

                    // Assign out[i] to bool2int(data_nibble).
                    out[i] = bool2int(data_nibble);
                // Close the current scope block.
                }
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
