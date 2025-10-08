// Only compile the following section when HAVE_CONFIG_H is defined.
#ifdef HAVE_CONFIG_H
// Bring in declarations from "config.h".
#include "config.h"
// Close the preceding conditional compilation block.
#endif

// Bring in declarations from "modulate_impl.h".
#include "modulate_impl.h"

// Start namespace gr scope.
namespace gr
// Open a new scope block.
{
    // Start namespace lora_sdr scope.
    namespace lora_sdr
    // Open a new scope block.
    {

        // Specify the shared pointer typedef associated with modulate.
        modulate::sptr
        // Define the e::make function.
        modulate::make(uint8_t sf, uint32_t samp_rate, uint32_t bw, std::vector<uint16_t> sync_words, uint32_t frame_zero_padd, uint16_t preamble_len = 8)
        // Open a new scope block.
        {
            // Return gnuradio::get_initial_sptr(new modulate_impl(sf, samp_rate, bw, sync_words, frame_zero_padd, preamble_len)) to the caller.
            return gnuradio::get_initial_sptr(new modulate_impl(sf, samp_rate, bw, sync_words, frame_zero_padd, preamble_len));
        // Close the current scope block.
        }
        /*
     * The private constructor
     */
        // Define the l::modulate_impl function.
        modulate_impl::modulate_impl(uint8_t sf, uint32_t samp_rate, uint32_t bw, std::vector<uint16_t> sync_words, uint32_t frame_zero_padd, uint16_t preamble_len)
            // Initialize base classes or members with gr::block("modulate",.
            : gr::block("modulate",
                        // Specify parameter or initializer gr::io_signature::make(1, 1, sizeof(uint32_t)).
                        gr::io_signature::make(1, 1, sizeof(uint32_t)),
                        // Define the e::make function.
                        gr::io_signature::make(1, 1, sizeof(gr_complex)))
        // Open a new scope block.
        {
            // Assign m_sf to sf.
            m_sf = sf;
            // Assign m_samp_rate to samp_rate.
            m_samp_rate = samp_rate;
            // Assign m_bw to bw.
            m_bw = bw;
            // Assign m_sync_words to sync_words.
            m_sync_words = sync_words;

            // Assign m_number_of_bins to (uint32_t)(1u << m_sf).
            m_number_of_bins = (uint32_t)(1u << m_sf);
            // Assign m_os_factor to m_samp_rate / m_bw.
            m_os_factor = m_samp_rate / m_bw;
            // Assign m_samples_per_symbol to (uint32_t)(m_number_of_bins*m_os_factor).
            m_samples_per_symbol = (uint32_t)(m_number_of_bins*m_os_factor);
            // Assign m_ninput_items_required to 1.
            m_ninput_items_required = 1;

            // Assign m_inter_frame_padding to frame_zero_padd. (inline comment notes: add some empty samples at the end of a frame important for transmission with LimeSDR Mini or simulation)
            m_inter_frame_padding = frame_zero_padd; // add some empty samples at the end of a frame important for transmission with LimeSDR Mini or simulation

            // Resize m_downchirp to m_samples_per_symbol.
            m_downchirp.resize(m_samples_per_symbol);
            // Resize m_upchirp to m_samples_per_symbol.
            m_upchirp.resize(m_samples_per_symbol);

            // Assign frame_end to true.
            frame_end = true;

            // Call build_ref_chirps with arguments (&m_upchirp[0], &m_downchirp[0], m_sf,m_os_factor).
            build_ref_chirps(&m_upchirp[0], &m_downchirp[0], m_sf,m_os_factor);

            //Convert given sync word into the two modulated values in preamble
            // Branch when condition (m_sync_words.size()==1) evaluates to true.
            if(m_sync_words.size()==1){
                // Assign uint16_t tmp to m_sync_words[0].
                uint16_t tmp = m_sync_words[0];
                // Resize m_sync_words to 2,0.
                m_sync_words.resize(2,0);
                // Assign m_sync_words[0] to ((tmp&0xF0)>>4)<<3.
                m_sync_words[0] = ((tmp&0xF0)>>4)<<3;
                // Assign m_sync_words[1] to (tmp&0x0F)<<3.
                m_sync_words[1] = (tmp&0x0F)<<3;
            // Close the current scope block.
            }
            // Branch when condition (preamble_len<5) evaluates to true.
            if (preamble_len<5)
            // Open a new scope block.
            {
               // Declare std::cerr<<RED<<" Preamble length should be greater than 5!"<<RESET<<std::endl.
               std::cerr<<RED<<" Preamble length should be greater than 5!"<<RESET<<std::endl;
            // Close the current scope block.
            }
            // Assign m_preamb_len to preamble_len.
            m_preamb_len = preamble_len;
            // Assign samp_cnt to -1.
            samp_cnt = -1;
            // Assign preamb_samp_cnt to 0.
            preamb_samp_cnt = 0;
            // Assign frame_cnt to 0.
            frame_cnt = 0;
            // Assign padd_cnt to m_inter_frame_padding.
            padd_cnt = m_inter_frame_padding;

            // Call set_tag_propagation_policy with arguments (TPP_DONT).
            set_tag_propagation_policy(TPP_DONT);
            // Call set_output_multiple with arguments (m_samples_per_symbol).
            set_output_multiple(m_samples_per_symbol);
        // Close the current scope block.
        }

        /*
     * Our virtual destructor.
     */
        // Define the l::~modulate_impl function.
        modulate_impl::~modulate_impl()
        // Open a new scope block.
        {
        // Close the current scope block.
        }

        // Execute void.
        void
        // Define the l::forecast function.
        modulate_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required)
        // Open a new scope block.
        {
            // Assign ninput_items_required[0] to m_ninput_items_required.
            ninput_items_required[0] = m_ninput_items_required;
        // Close the current scope block.
        }

        // Define the l::update_var function.
        void modulate_impl::update_var(int new_sf, int new_bw)
        // Open a new scope block.
        {
            // Branch when condition (new_sf != m_sf) evaluates to true.
            if (new_sf != m_sf) {
                // Assign m_sf to new_sf.
                m_sf = new_sf;
                // Execute statement std::cout<<"New sf Modulate "<< static_cast<int>(m_sf) <<std::endl.
                std::cout<<"New sf Modulate "<< static_cast<int>(m_sf) <<std::endl;
            // Close the current scope block.
            }
            // Branch when condition (new_bw != m_bw) evaluates to true.
            if (new_bw != m_bw) {
                // Assign m_bw to new_bw.
                m_bw = new_bw;
                // Execute statement std::cout<<"New bw Modulate "<< static_cast<int>(m_bw) <<std::endl.
                std::cout<<"New bw Modulate "<< static_cast<int>(m_bw) <<std::endl;
                // Assign m_samp_rate to 4 * m_bw.
                m_samp_rate = 4 * m_bw;
                // Execute statement std::cout<<"New samp rate Modulate "<< static_cast<int>(m_bw) <<std::endl.
                std::cout<<"New samp rate Modulate "<< static_cast<int>(m_bw) <<std::endl;
            // Close the current scope block.
            }
            // Assign m_number_of_bins to (uint32_t)(1u << m_sf).
            m_number_of_bins = (uint32_t)(1u << m_sf);
            // Assign m_os_factor to m_samp_rate / m_bw.
            m_os_factor = m_samp_rate / m_bw;
            // Assign m_samples_per_symbol to (uint32_t)(m_number_of_bins*m_os_factor).
            m_samples_per_symbol = (uint32_t)(m_number_of_bins*m_os_factor);
            // Assign m_ninput_items_required to 1.
            m_ninput_items_required = 1;

            //m_inter_frame_padding = frame_zero_padd; // add some empty samples at the end of a frame important for transmission with LimeSDR Mini or simulation

            // Resize m_downchirp to m_samples_per_symbol.
            m_downchirp.resize(m_samples_per_symbol);
            // Resize m_upchirp to m_samples_per_symbol.
            m_upchirp.resize(m_samples_per_symbol);


            // Call build_ref_chirps with arguments (&m_upchirp[0], &m_downchirp[0], m_sf,m_os_factor).
            build_ref_chirps(&m_upchirp[0], &m_downchirp[0], m_sf,m_os_factor);

            // Call set_output_multiple with arguments (m_samples_per_symbol).
            set_output_multiple(m_samples_per_symbol);
        // Close the current scope block.
        }

        // Specify parameter or initializer int modulate_impl::general_work(int noutput_items.
        int modulate_impl::general_work(int noutput_items,
                                        // Specify parameter or initializer gr_vector_int &ninput_items.
                                        gr_vector_int &ninput_items,
                                        // Specify parameter or initializer gr_vector_const_void_star &input_items.
                                        gr_vector_const_void_star &input_items,
                                        // Execute gr_vector_void_star &output_items).
                                        gr_vector_void_star &output_items)
        // Open a new scope block.
        {
            // Assign const uint32_t *in to (const uint32_t *)input_items[0].
            const uint32_t *in = (const uint32_t *)input_items[0];
            // Assign gr_complex *out to (gr_complex *)output_items[0].
            gr_complex *out = (gr_complex *)output_items[0];
            // Assign int nitems_to_process to ninput_items[0].
            int nitems_to_process = ninput_items[0];
            // Assign int output_offset to 0.
            int output_offset = 0;
            // read tags
            // Declare std::vector<tag_t> tags.
            std::vector<tag_t> tags;
            // Call get_tags_in_window with arguments (tags, 0, 0, ninput_items[0], pmt::string_to_symbol("frame_len")).
            get_tags_in_window(tags, 0, 0, ninput_items[0], pmt::string_to_symbol("frame_len"));
            // Branch when condition (tags.size()) evaluates to true.
            if (tags.size())
            // Open a new scope block.
            {
                // Branch when condition (tags[0].offset != nitems_read(0)) evaluates to true.
                if (tags[0].offset != nitems_read(0)){
                    // Assign nitems_to_process to std::min(tags[0].offset - nitems_read(0), (uint64_t)(float)noutput_items / m_samples_per_symbol).
                    nitems_to_process = std::min(tags[0].offset - nitems_read(0), (uint64_t)(float)noutput_items / m_samples_per_symbol);
                // Close the current scope block.
                }
                // Handle the alternative branch when previous conditions fail.
                else
                // Open a new scope block.
                {
                    // Branch when condition (tags.size() >= 2) evaluates to true.
                    if (tags.size() >= 2)
                        // Assign nitems_to_process to std::min(tags[1].offset - tags[0].offset, (uint64_t)(float)noutput_items / m_samples_per_symbol).
                        nitems_to_process = std::min(tags[1].offset - tags[0].offset, (uint64_t)(float)noutput_items / m_samples_per_symbol);
                    // Branch when condition (frame_end) evaluates to true.
                    if (frame_end)
                    // Open a new scope block.
                    {
                        // Assign m_frame_len to pmt::to_long(tags[0].value).
                        m_frame_len = pmt::to_long(tags[0].value);
                        // Assign m_framelen_tag to tags[0].
                        m_framelen_tag = tags[0];
                        // Call get_tags_in_window with arguments (tags, 0, 0, 1, pmt::string_to_symbol("configuration")).
                        get_tags_in_window(tags, 0, 0, 1, pmt::string_to_symbol("configuration"));
                        // Branch when condition (tags.size()>0) evaluates to true.
                        if(tags.size()>0)
                        // Open a new scope block.
                        {
                            // Assign m_config_tag to tags[0].
                            m_config_tag = tags[0];
                            // Assign m_config_tag.offset to nitems_written(0).
                            m_config_tag.offset = nitems_written(0); 

                            // Assign pmt::pmt_t err_sf to pmt::string_to_symbol("error").
                            pmt::pmt_t err_sf = pmt::string_to_symbol("error");
                            // Assign pmt::pmt_t err_bw to pmt::string_to_symbol("error").
                            pmt::pmt_t err_bw = pmt::string_to_symbol("error");
                            // Assign int new_sf to pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("sf"), err_sf)).
                            int new_sf = pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("sf"), err_sf));
                            // Assign int new_bw to pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("bw"), err_bw)).
                            int new_bw = pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("bw"), err_bw));
                            // Call update_var with arguments (new_sf, new_bw).
                            update_var(new_sf, new_bw);

                            // Call add_item_tag with arguments (0, m_config_tag).
                            add_item_tag(0, m_config_tag);
                        // Close the current scope block.
                        }

                        // Assign m_framelen_tag.offset to nitems_written(0).
                        m_framelen_tag.offset = nitems_written(0);

                        // Assign m_framelen_tag.value to pmt::from_long(int((m_frame_len + m_preamb_len + 4.25) * m_samples_per_symbol + m_inter_frame_padding )).
                        m_framelen_tag.value = pmt::from_long(int((m_frame_len + m_preamb_len + 4.25) * m_samples_per_symbol + m_inter_frame_padding ));

                        // Call add_item_tag with arguments (0, m_framelen_tag).
                        add_item_tag(0, m_framelen_tag);                

                        // Assign samp_cnt to -1.
                        samp_cnt = -1;
                        // Assign preamb_samp_cnt to 0.
                        preamb_samp_cnt = 0;
                        // Assign padd_cnt to 0.
                        padd_cnt = 0;
                        // Assign frame_end to false.
                        frame_end = false;
                    // Close the current scope block.
                    }
                // Close the current scope block.
                }
            // Close the current scope block.
            }

            // Branch when condition (samp_cnt == -1) evaluates to true. (inline comment notes: preamble)
            if (samp_cnt == -1) // preamble
            // Open a new scope block.
            {
                
                // Iterate with loop parameters (int i = 0; i < noutput_items / m_samples_per_symbol; i++).
                for (int i = 0; i < noutput_items / m_samples_per_symbol; i++)
                // Open a new scope block.
                {
                    // Branch when condition (preamb_samp_cnt < (m_preamb_len + 5)*m_samples_per_symbol) evaluates to true. (inline comment notes: should output preamble part)
                    if (preamb_samp_cnt < (m_preamb_len + 5)*m_samples_per_symbol) //should output preamble part
                    // Open a new scope block.
                    {
                        // Branch when condition (preamb_samp_cnt < (m_preamb_len*m_samples_per_symbol)) evaluates to true.
                        if (preamb_samp_cnt < (m_preamb_len*m_samples_per_symbol))
                        // Open a new scope block. (inline comment notes: upchirps)
                        { //upchirps
                            // Call memcpy with arguments (&out[output_offset], &m_upchirp[0], m_samples_per_symbol * sizeof(gr_complex)).
                            memcpy(&out[output_offset], &m_upchirp[0], m_samples_per_symbol * sizeof(gr_complex));
                        // Close the current scope block.
                        }
                        // Define the f function. (inline comment notes: sync words)
                        else if (preamb_samp_cnt == (m_preamb_len*m_samples_per_symbol)) //sync words
                            // Call build_upchirp with arguments (&out[output_offset], m_sync_words[0], m_sf,m_os_factor).
                            build_upchirp(&out[output_offset], m_sync_words[0], m_sf,m_os_factor);
                        // Define the f function.
                        else if (preamb_samp_cnt == (m_preamb_len + 1)*m_samples_per_symbol)
                            // Call build_upchirp with arguments (&out[output_offset], m_sync_words[1], m_sf,m_os_factor).
                            build_upchirp(&out[output_offset], m_sync_words[1], m_sf,m_os_factor);
                        // Define the f function. (inline comment notes: 2.25 downchirps)
                        else if (preamb_samp_cnt < (m_preamb_len + 4)*m_samples_per_symbol) //2.25 downchirps
                            // Call memcpy with arguments (&out[output_offset], &m_downchirp[0], m_samples_per_symbol * sizeof(gr_complex)).
                            memcpy(&out[output_offset], &m_downchirp[0], m_samples_per_symbol * sizeof(gr_complex));
                        // Define the f function.
                        else if (preamb_samp_cnt == (m_preamb_len + 4)*m_samples_per_symbol)
                        // Open a new scope block.
                        {
                            // Call memcpy with arguments (&out[output_offset], &m_downchirp[0], m_samples_per_symbol / 4 * sizeof(gr_complex)).
                            memcpy(&out[output_offset], &m_downchirp[0], m_samples_per_symbol / 4 * sizeof(gr_complex));
                            //correct offset dur to quarter of downchirp
                            // Assign output_offset - to 3 * m_samples_per_symbol / 4.
                            output_offset -= 3 * m_samples_per_symbol / 4;
                            // Assign samp_cnt to 0.
                            samp_cnt = 0;
                            
                        // Close the current scope block.
                        }
                        // Assign output_offset + to m_samples_per_symbol.
                        output_offset += m_samples_per_symbol;
                        // Assign preamb_samp_cnt + to m_samples_per_symbol.
                        preamb_samp_cnt += m_samples_per_symbol;
                    // Close the current scope block.
                    }
                // Close the current scope block.
                }
            // Close the current scope block.
            }
            
            // Branch when condition ( samp_cnt < m_frame_len*(int32_t)m_samples_per_symbol && samp_cnt>-1) evaluates to true. (inline comment notes: output payload)
            if ( samp_cnt < m_frame_len*(int32_t)m_samples_per_symbol && samp_cnt>-1) //output payload
            // Open a new scope block.
            {
                // Assign nitems_to_process to std::min(nitems_to_process, int((float)(noutput_items - output_offset) / m_samples_per_symbol)).
                nitems_to_process = std::min(nitems_to_process, int((float)(noutput_items - output_offset) / m_samples_per_symbol));
                // Assign nitems_to_process to std::min(nitems_to_process, ninput_items[0]).
                nitems_to_process = std::min(nitems_to_process, ninput_items[0]);
                // Iterate with loop parameters (int i = 0; i < nitems_to_process; i++).
                for (int i = 0; i < nitems_to_process; i++)
                // Open a new scope block.
                {
                    // Call build_upchirp with arguments (&out[output_offset], in[i], m_sf,m_os_factor).
                    build_upchirp(&out[output_offset], in[i], m_sf,m_os_factor);
                    // Assign output_offset + to m_samples_per_symbol.
                    output_offset += m_samples_per_symbol;
                    // Assign samp_cnt + to m_samples_per_symbol.
                    samp_cnt += m_samples_per_symbol;
                // Close the current scope block.
                }
            // Close the current scope block.
            }
            // Handle the alternative branch when previous conditions fail.
            else
            // Open a new scope block.
            {
                // Assign nitems_to_process to 0.
                nitems_to_process = 0;
            // Close the current scope block.
            }

            // Branch when condition ((samp_cnt >= (m_frame_len*m_samples_per_symbol)) evaluates to true.
            if ((samp_cnt >= (m_frame_len*m_samples_per_symbol)) && 
                // Evaluate expression (samp_cnt < m_frame_len*m_samples_per_symbol + (int64_t)m_inter_frame_padding)). (inline comment notes: padd frame end with zeros)
                (samp_cnt < m_frame_len*m_samples_per_symbol + (int64_t)m_inter_frame_padding)) //padd frame end with zeros
            // Open a new scope block.
            {
                // Assign m_ninput_items_required to 0.
                m_ninput_items_required = 0;
                // Assign int padd_size to std::min(uint32_t(noutput_items - output_offset), m_frame_len*m_samples_per_symbol + m_inter_frame_padding - samp_cnt ).
                int padd_size = std::min(uint32_t(noutput_items - output_offset), m_frame_len*m_samples_per_symbol + m_inter_frame_padding - samp_cnt );
                // Call fill with arguments (out+output_offset, out+output_offset+padd_size, gr_complex(0.0, 0.0)).
                fill(out+output_offset, out+output_offset+padd_size, gr_complex(0.0, 0.0));
                // Assign samp_cnt + to padd_size.
                samp_cnt += padd_size;
                // Assign padd_cnt + to padd_size.
                padd_cnt += padd_size;
                // Assign output_offset + to padd_size.
                output_offset += padd_size;
                // for (int i = 0; i < (noutput_items - output_offset); i++)
                // {
                //     if (samp_cnt < m_frame_len*m_samples_per_symbol + m_inter_frame_padding)
                //     {

                //         out[output_offset + i] = gr_complex(0.0, 0.0);
                //         output_offset += m_samples_per_symbol;
                //         symb_cnt++;
                //         padd_cnt++;
                //     }
                // }
            // Close the current scope block.
            }
            // Branch when condition ( samp_cnt == m_frame_len*m_samples_per_symbol + (int64_t)m_inter_frame_padding) evaluates to true.
            if ( samp_cnt == m_frame_len*m_samples_per_symbol + (int64_t)m_inter_frame_padding)
            // Open a new scope block.
            {
                // Increment samp_cnt.
                samp_cnt++;
                // Increment frame_cnt.
                frame_cnt++;
                // Assign m_ninput_items_required to 1.
                m_ninput_items_required = 1;
                // Assign frame_end to true.
                frame_end = true;
            
// Only compile the following section when GR_LORA_PRINT_INFO is defined.
#ifdef GR_LORA_PRINT_INFO              
                // Declare std::cout << "Frame " << frame_cnt << " sent\n".
                std::cout << "Frame " << frame_cnt << " sent\n";
// Close the preceding conditional compilation block.
#endif
            // Close the current scope block.
            }
            // if (nitems_to_process)
            //     std::cout << ninput_items[0] << " " << nitems_to_process << " " << output_offset << " " << noutput_items << std::endl;
            // Call consume_each with arguments (nitems_to_process).
            consume_each(nitems_to_process);
            // Return output_offset to the caller.
            return output_offset;
        // Close the current scope block.
        }

    // Close the current scope and emit the trailing comment.
    } /* namespace lora */
// Close the current scope and emit the trailing comment.
} /* namespace gr */
