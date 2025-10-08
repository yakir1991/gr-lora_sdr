// Only compile the following section when HAVE_CONFIG_H is defined.
#ifdef HAVE_CONFIG_H
// Bring in declarations from "config.h".
#include "config.h"
// Close the preceding conditional compilation block.
#endif

// Bring in declarations from <gnuradio/io_signature.h>.
#include <gnuradio/io_signature.h>
// Bring in declarations from <chrono>.
#include <chrono>
// Bring in declarations from "crc_verif_impl.h".
#include "crc_verif_impl.h"

// Bring in declarations from <gnuradio/lora_sdr/utilities.h>. (inline comment notes: for print color)
#include <gnuradio/lora_sdr/utilities.h> // for print color
// Bring in declarations from "gnuradio/lora_sdr/crc_verif.h".
#include "gnuradio/lora_sdr/crc_verif.h"

// Start namespace gr scope.
namespace gr
// Open a new scope block.
{
    // Start namespace lora_sdr scope.
    namespace lora_sdr
    // Open a new scope block.
    {

        // Specify the shared pointer typedef associated with crc_verif.
        crc_verif::sptr
        // Define the f::make function.
        crc_verif::make(int print_rx_msg, bool output_crc_check)
        // Open a new scope block.
        {
            // Return gnuradio::get_initial_sptr(new crc_verif_impl(print_rx_msg, output_crc_check)) to the caller.
            return gnuradio::get_initial_sptr(new crc_verif_impl(print_rx_msg, output_crc_check));
        // Close the current scope block.
        }

        /*
         * The private constructor
         */
        // Define the l::crc_verif_impl function.
        crc_verif_impl::crc_verif_impl(int print_rx_msg, bool output_crc_check)
            // Initialize base classes or members with gr::block("crc_verif",.
            : gr::block("crc_verif",
                        // Specify parameter or initializer gr::io_signature::make(1, 1, sizeof(uint8_t)).
                        gr::io_signature::make(1, 1, sizeof(uint8_t)),
                        // Specify parameter or initializer gr::io_signature::make2(0, 2, sizeof(uint8_t), sizeof(uint8_t))).
                        gr::io_signature::make2(0, 2, sizeof(uint8_t), sizeof(uint8_t))),
                        // Specify parameter or initializer print_rx_msg(print_rx_msg).
                        print_rx_msg(print_rx_msg),
                  // Define the k function.
                  output_crc_check(output_crc_check)
        // Open a new scope block.
        {
            // Call message_port_register_out with arguments (pmt::mp("msg")).
            message_port_register_out(pmt::mp("msg"));
            // Call set_tag_propagation_policy with arguments (TPP_DONT).
            set_tag_propagation_policy(TPP_DONT);
            
        // Close the current scope block.
        }

        /*
         * Our virtual destructor.
         */
        // Define the l::~crc_verif_impl function.
        crc_verif_impl::~crc_verif_impl()
        // Open a new scope block.
        {
        // Close the current scope block.
        }

        // Define the l::forecast function.
        void crc_verif_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required)
        // Open a new scope block.
        {
            // Assign ninput_items_required[0] to 1. (inline comment notes: m_payload_len;)
            ninput_items_required[0] = 1; // m_payload_len;
        // Close the current scope block.
        }
        // Define the l::crc16 function.
        unsigned int crc_verif_impl::crc16(uint8_t *data, uint32_t len)
        // Open a new scope block.
        {

            // Assign uint16_t crc to 0x0000.
            uint16_t crc = 0x0000;
            // Iterate with loop parameters (unsigned int i = 0; i < len; i++).
            for (unsigned int i = 0; i < len; i++)
            // Open a new scope block.
            {
                // Assign uint8_t newByte to data[i].
                uint8_t newByte = data[i];

                // Iterate with loop parameters (unsigned char i = 0; i < 8; i++).
                for (unsigned char i = 0; i < 8; i++)
                // Open a new scope block.
                {
                    // Branch when condition (((crc & 0x8000) >> 8) ^ (newByte & 0x80)) evaluates to true.
                    if (((crc & 0x8000) >> 8) ^ (newByte & 0x80))
                    // Open a new scope block.
                    {
                        // Assign crc to (crc << 1) ^ 0x1021.
                        crc = (crc << 1) ^ 0x1021;
                    // Close the current scope block.
                    }
                    // Handle the alternative branch when previous conditions fail.
                    else
                    // Open a new scope block.
                    {
                        // Assign crc to (crc << 1).
                        crc = (crc << 1);
                    // Close the current scope block.
                    }
                    // Assign newByte << to 1.
                    newByte <<= 1;
                // Close the current scope block.
                }
            // Close the current scope block.
            }
            // Return crc to the caller.
            return crc;
        // Close the current scope block.
        }

        // Specify parameter or initializer int crc_verif_impl::general_work(int noutput_items.
        int crc_verif_impl::general_work(int noutput_items,
                                         // Specify parameter or initializer gr_vector_int &ninput_items.
                                         gr_vector_int &ninput_items,
                                         // Specify parameter or initializer gr_vector_const_void_star &input_items.
                                         gr_vector_const_void_star &input_items,
                                         // Execute gr_vector_void_star &output_items).
                                         gr_vector_void_star &output_items)
        // Open a new scope block.
        {
            // Assign uint8_t *in to (uint8_t *)input_items[0].
            uint8_t *in = (uint8_t *)input_items[0];
            // Declare uint8_t *out.
            uint8_t *out;
            // Declare bool *out_crc.
            bool *out_crc;
            // Branch when condition (output_items.size()) evaluates to true.
            if (output_items.size())
            // Open a new scope block.
            {
                // Assign out to (uint8_t *)output_items[0].
                out = (uint8_t *)output_items[0];
                // Branch when condition (output_crc_check) evaluates to true.
                if (output_crc_check)
                    // Assign out_crc to (bool *)output_items[1].
                    out_crc = (bool *)output_items[1]; 
            // Close the current scope block.
            }

            // Declare std::vector<tag_t> tags.
            std::vector<tag_t> tags;
            // Call get_tags_in_window with arguments (tags, 0, 0, ninput_items[0], pmt::string_to_symbol("frame_info")).
            get_tags_in_window(tags, 0, 0, ninput_items[0], pmt::string_to_symbol("frame_info"));
            // Branch when condition (tags.size()) evaluates to true.
            if (tags.size())
            // Open a new scope block.
            {
                // Assign pmt::pmt_t err to pmt::string_to_symbol("error").
                pmt::pmt_t err = pmt::string_to_symbol("error");
                // Assign m_crc_presence to pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("crc"), err)).
                m_crc_presence = pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("crc"), err));
                // Assign m_payload_len to pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("pay_len"), err)).
                m_payload_len = pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("pay_len"), err));
                // Assign current_tag to tags[0].
                current_tag = tags[0];
                // std::cout<<m_payload_len<<" "<<nitem_to_process<<std::endl;
                // std::cout<<"\ncrc_crc "<<tags[0].offset<<" - crc: "<<(int)m_crc_presence<<" - pay_len: "<<(int)m_payload_len<<"\n";
                
                
            // Close the current scope block.
            }
            //append received bytes to buffer
            // Iterate with loop parameters (int i = 0; i < ninput_items[0]; i++).
            for (int i = 0; i < ninput_items[0]; i++)
            // Open a new scope block.
            {
                // Append in[i] to in_buff.
                in_buff.push_back(in[i]);
            // Close the current scope block.
            }
            // Call consume_each with arguments (ninput_items[0]).
            consume_each(ninput_items[0]);


            // Branch when condition ((in_buff.size() >= m_payload_len + 2) && m_crc_presence) evaluates to true.
            if ((in_buff.size() >= m_payload_len + 2) && m_crc_presence)
            // Open a new scope block. (inline comment notes: wait for all the payload to come)
            { // wait for all the payload to come

                // Branch when condition (m_payload_len < 2) evaluates to true.
                if (m_payload_len < 2)
                // Open a new scope block. (inline comment notes: undefined CRC)
                { // undefined CRC
                    // Declare std::cout << "CRC not supported for payload smaller than 2 bytes" << std::endl.
                    std::cout << "CRC not supported for payload smaller than 2 bytes" << std::endl;
                    // Return 0 to the caller.
                    return 0;
                // Close the current scope block.
                }
                // Handle the alternative branch when previous conditions fail.
                else
                // Open a new scope block.
                {
                    // calculate CRC on the N-2 firsts data bytes
                    // Assign m_crc to crc16(&in_buff[0], m_payload_len - 2).
                    m_crc = crc16(&in_buff[0], m_payload_len - 2);

                    // XOR the obtained CRC with the last 2 data bytes
                    // Assign m_crc to m_crc ^ in_buff[m_payload_len - 1] ^ (in_buff[m_payload_len - 2] << 8).
                    m_crc = m_crc ^ in_buff[m_payload_len - 1] ^ (in_buff[m_payload_len - 2] << 8);
// Only compile the following section when GRLORA_DEBUG is defined.
#ifdef GRLORA_DEBUG
                    // Iterate with loop parameters (int i = 0; i < (int)m_payload_len + 2; i++).
                    for (int i = 0; i < (int)m_payload_len + 2; i++)
                        // Execute statement std::cout << std::hex << (int)in_buff[i] << std::dec << std::endl.
                        std::cout << std::hex << (int)in_buff[i] << std::dec << std::endl;
                    // Declare std::cout << "Calculated " << std::hex << m_crc << std::dec << std::endl.
                    std::cout << "Calculated " << std::hex << m_crc << std::dec << std::endl;
                    // Execute statement std::cout << "Got " << std::hex << (in_buff[m_payload_len] + (in_buff[m_payload_len + 1] << 8)) << std::dec << std::endl.
                    std::cout << "Got " << std::hex << (in_buff[m_payload_len] + (in_buff[m_payload_len + 1] << 8)) << std::dec << std::endl;
// Close the preceding conditional compilation block.
#endif

                    // get payload as string
                    // Clear all contents from message_str.
                    message_str.clear();
                    // Iterate with loop parameters (int i = 0; i < (int)m_payload_len; i++).
                    for (int i = 0; i < (int)m_payload_len; i++)
                    // Open a new scope block.
                    {
                        // Assign m_char to (char)in_buff[i].
                        m_char = (char)in_buff[i];
                        // Assign message_str to message_str + m_char.
                        message_str = message_str + m_char;
                        // Branch when condition (output_items.size()) evaluates to true.
                        if (output_items.size())
                            // Assign out[i] to in_buff[i].
                            out[i] = in_buff[i];
                    // Close the current scope block.
                    }
                    // Increment cnt.
                    cnt++;
                    // Declare uint8_t crc_valid.
                    uint8_t crc_valid;
                    // Branch when condition (!(in_buff[m_payload_len] + (in_buff[m_payload_len + 1] << 8) - m_crc)) evaluates to true.
                    if (!(in_buff[m_payload_len] + (in_buff[m_payload_len + 1] << 8) - m_crc))
                        // Assign crc_valid to 1.
                        crc_valid = 1;
                    // Handle the alternative branch when previous conditions fail.
                    else
                        // Assign crc_valid to 0.
                        crc_valid = 0;
                    
                    // Branch when condition (output_crc_check) evaluates to true.
                    if(output_crc_check){
                        // Assign out_crc[0] to crc_valid.
                        out_crc[0] = crc_valid;
                        // Call produce with arguments (1,1).
                        produce(1,1);
                    // Close the current scope block.
                    }
// Branch when condition (output_items.size()) evaluates to true.
					if (output_items.size()){
// Assign current_tag.value to pmt::dict_add(current_tag.value, pmt::string_to_symbol("crc_valid"), pmt::from_bool(crc_valid == 1)).
		                current_tag.value = pmt::dict_add(current_tag.value, pmt::string_to_symbol("crc_valid"), pmt::from_bool(crc_valid == 1));
// Assign current_tag.offset to nitems_written(0).
		                current_tag.offset = nitems_written(0);
// Call add_item_tag with arguments (0, current_tag).
		                add_item_tag(0, current_tag);
                    // Close the current scope block.
                    }
                    // Branch when condition (print_rx_msg != NONE) evaluates to true.
                    if (print_rx_msg != NONE)
                    // Open a new scope block.
                    {
                        // Branch when condition (print_rx_msg == ASCII) evaluates to true.
                        if(print_rx_msg == ASCII)
                            // Declare std::cout << "rx msg: " << message_str << std::endl.
                            std::cout << "rx msg: " << message_str << std::endl;                        
                        // Define the f function.
                        else if(print_rx_msg == HEX){
                            // Declare std::cout << "rx msg: ".
                            std::cout << "rx msg: ";
                            // Iterate with loop parameters (int i = 0; i < (int)m_payload_len; i++).
                            for (int i = 0; i < (int)m_payload_len; i++){
                                // Execute statement std::cout << std::hex <<"0x"<< (int)in_buff[i] << std::dec.
                                std::cout << std::hex <<"0x"<< (int)in_buff[i] << std::dec;
                                // Branch when condition (i != (int)m_payload_len-1) evaluates to true.
                                if(i != (int)m_payload_len-1)
                                    // Declare std::cout << ", ".
                                    std::cout << ", ";
                            // Close the current scope block.
                            }
                            // Declare std::cout << std::endl.
                            std::cout << std::endl;
                        // Close the current scope block.
                        }

                        // Branch when condition (crc_valid) evaluates to true.
                        if (crc_valid)
                            // Invoke std::cout << "CRC valid!" << std::endl.
                            std::cout << "CRC valid!" << std::endl
                                      // Declare << std::endl.
                                      << std::endl;
                        // Handle the alternative branch when previous conditions fail.
                        else
                            // Invoke std::cout << RED << "CRC invalid" << RESET << std::endl.
                            std::cout << RED << "CRC invalid" << RESET << std::endl
                                      // Declare << std::endl.
                                      << std::endl;
                    // Close the current scope block.
                    }
                    // Call message_port_pub with arguments (pmt::intern("msg"), pmt::mp(message_str)).
                    message_port_pub(pmt::intern("msg"), pmt::mp(message_str));
                    // Remove range in_buff.begin(), in_buff.begin()+m_payload_len + 2 from in_buff.
                    in_buff.erase(in_buff.begin(), in_buff.begin()+m_payload_len + 2);
                    // Branch when condition (output_crc_check) evaluates to true.
                    if(output_crc_check){
                        // Call produce with arguments (0,m_payload_len).
                        produce(0,m_payload_len);
                        // Return WORK_CALLED_PRODUCE to the caller.
                        return WORK_CALLED_PRODUCE;
                    // Close the current scope block.
                    }
                    // Handle the alternative branch when previous conditions fail.
                    else
                        // Return m_payload_len to the caller.
                        return m_payload_len;
                // Close the current scope block.
                }
            // Close the current scope block.
            }
            // Define the f function.
            else if ((in_buff.size()>= m_payload_len) && !m_crc_presence)
            // Open a new scope block.
            {
                // Branch when condition (output_items.size()) evaluates to true.
                if (output_items.size()){
// Assign current_tag.offset to nitems_written(0).
		            current_tag.offset = nitems_written(0);
// Call add_item_tag with arguments (0, current_tag).
		            add_item_tag(0, current_tag);
// Close the current scope block.
				}
                // get payload as string
                // Clear all contents from message_str.
                message_str.clear();
                // Iterate with loop parameters (unsigned int i = 0; i < m_payload_len; i++).
                for (unsigned int i = 0; i < m_payload_len; i++)
                // Open a new scope block.
                {
                    // Assign m_char to (char)in_buff[i].
                    m_char = (char)in_buff[i];
                    // Assign message_str to message_str + m_char.
                    message_str = message_str + m_char;
                    // Branch when condition (output_items.size()) evaluates to true.
                    if (output_items.size())
                        // Assign out[i] to in_buff[i].
                        out[i] = in_buff[i];
                // Close the current scope block.
                }
                // Increment cnt.
                cnt++;
                // Remove range in_buff.begin(), in_buff.begin() + m_payload_len from in_buff.
                in_buff.erase(in_buff.begin(), in_buff.begin() + m_payload_len );
                // Branch when condition (print_rx_msg == ASCII) evaluates to true.
                if (print_rx_msg == ASCII)
                    // Declare std::cout << "rx msg: " << message_str << std::endl.
                    std::cout << "rx msg: " << message_str << std::endl;
                // Define the f function.
                else if(print_rx_msg == HEX){
                    // Declare std::cout << "rx msg: ".
                    std::cout << "rx msg: ";
                    // Iterate with loop parameters (int i = 0; i < (int)m_payload_len; i++).
                    for (int i = 0; i < (int)m_payload_len; i++){
                        // Execute statement std::cout << std::hex <<"0x"<< (int)in_buff[i]<< std::dec.
                        std::cout << std::hex <<"0x"<< (int)in_buff[i]<< std::dec;
                        // Branch when condition (i != (int)m_payload_len-1) evaluates to true.
                        if(i != (int)m_payload_len-1)
                            // Declare std::cout << ", ".
                            std::cout << ", ";
                    // Close the current scope block.
                    }
                    // Declare std::cout << std::endl.
                    std::cout << std::endl;
                // Close the current scope block.
                }
                // Call message_port_pub with arguments (pmt::intern("msg"), pmt::mp(message_str)).
                message_port_pub(pmt::intern("msg"), pmt::mp(message_str));
                
                // Return m_payload_len to the caller.
                return m_payload_len;
            // Close the current scope block.
            }
            // Handle the alternative branch when previous conditions fail.
            else
                // Return 0 to the caller.
                return 0;
            
        // Close the current scope block.
        }
    // Close the current scope and emit the trailing comment.
    } /* namespace lora */
// Close the current scope and emit the trailing comment.
} /* namespace gr */
