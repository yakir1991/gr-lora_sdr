

// Only compile the following section when HAVE_CONFIG_H is defined.
#ifdef HAVE_CONFIG_H
// Bring in declarations from "config.h".
#include "config.h"
// Close the preceding conditional compilation block.
#endif

// Bring in declarations from <gnuradio/io_signature.h>.
#include <gnuradio/io_signature.h>
// Bring in declarations from "whitening_impl.h".
#include "whitening_impl.h"
// Bring in declarations from "tables.h".
#include "tables.h"

// Start namespace gr scope.
namespace gr
// Open a new scope block.
{
    // Start namespace lora_sdr scope.
    namespace lora_sdr
    // Open a new scope block.
    {

        // Specify the shared pointer typedef associated with whitening.
        whitening::sptr
        // Define the g::make function.
        whitening::make(bool is_hex, bool use_length_tag, char separator, std::string length_tag_name)
        // Open a new scope block.
        {
            // Return gnuradio::get_initial_sptr(new whitening_impl(is_hex, use_length_tag, separator, length_tag_name)) to the caller.
            return gnuradio::get_initial_sptr(new whitening_impl(is_hex, use_length_tag, separator, length_tag_name));
        // Close the current scope block.
        }

        /*
         * The private constructor
         */
        // Define the l::whitening_impl function.
        whitening_impl::whitening_impl(bool is_hex, bool use_length_tag, char separator, std::string length_tag_name)
            // Initialize base classes or members with gr::sync_interpolator("whitening",.
            : gr::sync_interpolator("whitening",
                                    // Specify parameter or initializer gr::io_signature::make(0, 1, sizeof(uint8_t)).
                                    gr::io_signature::make(0, 1, sizeof(uint8_t)),
                                    // Define the e::make function.
                                    gr::io_signature::make(1, 1, sizeof(uint8_t)), is_hex ? 1 : 2)
        // Open a new scope block.
        {
            // Assign m_separator to separator.
            m_separator = separator;
            // Assign m_file_source to false.
            m_file_source = false;
            // Assign m_use_length_tag to use_length_tag.
            m_use_length_tag = use_length_tag;
            // Assign m_length_tag_name to length_tag_name.
            m_length_tag_name = length_tag_name;
            // Assign m_is_hex to use_length_tag?false:is_hex. (inline comment notes: cant use length tag if input is given as a string of hex values)
            m_is_hex = use_length_tag?false:is_hex;// cant use length tag if input is given as a string of hex values
            // Assign m_tag_offset to 1.
            m_tag_offset = 1;

            // Call message_port_register_in with arguments (pmt::mp("dict")).
            message_port_register_in(pmt::mp("dict"));
            // Define the r function.
            set_msg_handler(pmt::mp("dict"), [this](pmt::pmt_t dict)
                            // Execute statement { this->frame_info_handler(dict); }).
                            { this->frame_info_handler(dict); });

            // Call message_port_register_in with arguments (pmt::mp("msg")).
            message_port_register_in(pmt::mp("msg"));
            // Define the r function.
            set_msg_handler(pmt::mp("msg"), [this](pmt::pmt_t msg)
                            // Execute statement { this->msg_handler(msg); }).
                            { this->msg_handler(msg); });   
        // Close the current scope block.
        }

        /*
         * Our virtual destructor.
         */
        // Define the l::~whitening_impl function.
        whitening_impl::~whitening_impl() {}

        // Define the l::msg_handler function.
        void whitening_impl::msg_handler(pmt::pmt_t message)
        // Open a new scope block.
        {
            // Branch when condition (m_file_source) evaluates to true.
            if (m_file_source)
            // Open a new scope block.
            {
                // Declare std::cout << RED << "Whitening can't have both input used simultaneously" << RESET << std::endl.
                std::cout << RED << "Whitening can't have both input used simultaneously" << RESET << std::endl;
            // Close the current scope block.
            }
            //  payload_str.push_back(random_string(rand()%253+2));
            // payload_str.push_back(rand()%2?"12345":"abcdefghijklmnop");
            // Append pmt::symbol_to_string(message) to payload_str.
            payload_str.push_back(pmt::symbol_to_string(message));
        // Close the current scope block.
        }
        // Define the l::frame_info_handler function.
        void whitening_impl::frame_info_handler(pmt::pmt_t frame_info)
        // Open a new scope block.
        {
            // std::cout<<" info "<<std::endl;

            // Assign pmt::pmt_t err to pmt::string_to_symbol("error").
            pmt::pmt_t err = pmt::string_to_symbol("error");

            // Assign int m_cr to pmt::to_long(pmt::dict_ref(frame_info, pmt::string_to_symbol("cr"), err)).
            int m_cr = pmt::to_long(pmt::dict_ref(frame_info, pmt::string_to_symbol("cr"), err));
            // Execute statement std::cout<<m_cr<<std::endl.
            std::cout<<m_cr<<std::endl;

            // Call add_item_tag with arguments (0, nitems_written(0), pmt::string_to_symbol("configuration"), frame_info).
            add_item_tag(0, nitems_written(0), pmt::string_to_symbol("configuration"), frame_info);

        // Close the current scope block.
        }

        // Specify parameter or initializer int whitening_impl::work(int noutput_items.
        int whitening_impl::work(int noutput_items,
                                 // Specify parameter or initializer gr_vector_const_void_star &input_items.
                                 gr_vector_const_void_star &input_items,
                                 // Execute gr_vector_void_star &output_items).
                                 gr_vector_void_star &output_items)
        // Open a new scope block.
        {
            // std::lock_guard<std::mutex> lock(m_payload_mutex);
            // check if input file is used
            // Declare uint8_t *in.
            uint8_t *in;
            // Branch when condition (input_items.size()) evaluates to true.
            if (input_items.size())
            // Open a new scope block.
            {
                // Assign m_file_source to true.
                m_file_source = true;
                // Assign in to (uint8_t *)input_items[0].
                in = (uint8_t *)input_items[0];
                // Declare std::string s.
                std::string s;

                // Assign int nitem_to_process to noutput_items.
                int nitem_to_process = noutput_items;
                // Declare int m_frame_len.
                int m_frame_len;
                // Branch when condition (m_use_length_tag) evaluates to true.
                if (m_use_length_tag)
                // Open a new scope block.
                {
                    // search for tag
                    // Declare std::vector<tag_t> tags.
                    std::vector<tag_t> tags;
                    // Call get_tags_in_window with arguments (tags, 0, 0, noutput_items, pmt::string_to_symbol(m_length_tag_name)).
                    get_tags_in_window(tags, 0, 0, noutput_items, pmt::string_to_symbol(m_length_tag_name));
                    // Branch when condition (tags.size()) evaluates to true.
                    if (tags.size())
                    // Open a new scope block.
                    {
                        // process only until next tag
                        // Branch when condition (tags[0].offset != nitems_read(0)) evaluates to true.
                        if (tags[0].offset != nitems_read(0))
                        // Open a new scope block.
                        {
                            // Assign nitem_to_process to tags[0].offset - nitems_read(0).
                            nitem_to_process = tags[0].offset - nitems_read(0);
                        // Close the current scope block.
                        }
                        // Handle the alternative branch when previous conditions fail. (inline comment notes: new frame)
                        else // new frame
                        // Open a new scope block.
                        {
                            //only consider tags with a new offset value
                            // Branch when condition (tags[0].offset != m_tag_offset) evaluates to true.
                            if(tags[0].offset != m_tag_offset)
                            // Open a new scope block.
                            {
                                // Branch when condition (tags.size() >= 2) evaluates to true.
                                if (tags.size() >= 2)
                                // Open a new scope block.
                                {
                                    // Assign nitem_to_process to tags[1].offset - tags[0].offset.
                                    nitem_to_process = tags[1].offset - tags[0].offset;
                                // Close the current scope block.
                                }
                                // Assign m_frame_len to pmt::to_long(tags[0].value).
                                m_frame_len = pmt::to_long(tags[0].value);
                                // Assign m_tag_offset to tags[0].offset.
                                m_tag_offset = tags[0].offset;
                                // Assign m_input_byte_cnt to 0.
                                m_input_byte_cnt = 0;
                                // Declare std::cout << "[whitening_impl.cc] offset "<<tags[0].offset<<" New frame of length " << m_frame_len << std::endl.
                                std::cout << "[whitening_impl.cc] offset "<<tags[0].offset<<" New frame of length " << m_frame_len << std::endl;
                            // Close the current scope block.
                            }
                        // Close the current scope block.
                        }
                    // Close the current scope block.
                    }
                    // Iterate with loop parameters (int i = 0; i < nitem_to_process; i++). (inline comment notes: read payload)
                    for (int i = 0; i < nitem_to_process; i++) // read payload
                    // Open a new scope block.
                    {
                        // Append in[i] to s.
                        s.push_back(in[i]);
                        // Increment m_input_byte_cnt.
                        m_input_byte_cnt++;
                        // Branch when condition (m_input_byte_cnt == m_frame_len) evaluates to true.
                        if (m_input_byte_cnt == m_frame_len)
                        // Open a new scope block.
                        {
                            // Append s to payload_str.
                            payload_str.push_back(s);
                        // Close the current scope block.
                        }
                    // Close the current scope block.
                    }
                // Close the current scope block.
                }

                // Handle the alternative branch when previous conditions fail.
                else
                // Open a new scope block.
                {
                    // Iterate with loop parameters (int i = 0; i < noutput_items / (m_is_hex ? 1 : 2); i++). (inline comment notes: read payload)
                    for (int i = 0; i < noutput_items / (m_is_hex ? 1 : 2); i++) // read payload
                    // Open a new scope block.
                    {

                        // Branch when condition (in[i] == m_separator) evaluates to true.
                        if (in[i] == m_separator)
                        // Open a new scope block.
                        {
                            // Call consume_each with arguments (sizeof(m_separator)). (inline comment notes: consume the m_separator character)
                            consume_each(sizeof(m_separator)); // consume the m_separator character
                            // Append s to payload_str.
                            payload_str.push_back(s);
                            // Exit the nearest enclosing loop or switch.
                            break;
                        // Close the current scope block.
                        }
                        // Append in[i] to s.
                        s.push_back(in[i]);
                    // Close the current scope block.
                    }
                // Close the current scope block.
                }
            // Close the current scope block.
            }

            // check if too many messages queued by the message strobe source
            // Branch when condition (payload_str.size() >= 100 && !(payload_str.size() % 100) && !m_file_source) evaluates to true.
            if (payload_str.size() >= 100 && !(payload_str.size() % 100) && !m_file_source)
            // Open a new scope block.
            {
                // Execute statement std::cout << RED << payload_str.size() << " frames in waiting list. Transmitter has issue to keep up at that transmission frequency." << RESET << std::endl.
                std::cout << RED << payload_str.size() << " frames in waiting list. Transmitter has issue to keep up at that transmission frequency." << RESET << std::endl;
            // Close the current scope block.
            }
            // Branch when condition (payload_str.size() && (uint32_t)noutput_items >= (m_is_hex ? 1 : 2) * payload_str.front().length()) evaluates to true.
            if (payload_str.size() && (uint32_t)noutput_items >= (m_is_hex ? 1 : 2) * payload_str.front().length())
            // Open a new scope block.
            {
                // Assign uint8_t *out to (uint8_t *)output_items[0].
                uint8_t *out = (uint8_t *)output_items[0];
                // Branch when condition (m_is_hex) evaluates to true.
                if (m_is_hex)
                // Open a new scope block.
                {
                    // Assign int len to payload_str.front().length().
                    int len = payload_str.front().length();
                    // Declare std::string newString.
                    std::string newString;
                    // Iterate with loop parameters (int i = 0; i < len; i += 2).
                    for (int i = 0; i < len; i += 2)
                    // Open a new scope block.
                    {
                        // Assign std::string byte to payload_str.front().substr(i, 2).
                        std::string byte = payload_str.front().substr(i, 2);
                        // Assign char chr to (char)(int)strtol(byte.c_str(), NULL, 16).
                        char chr = (char)(int)strtol(byte.c_str(), NULL, 16);
                        // Append chr to newString.
                        newString.push_back(chr);
                    // Close the current scope block.
                    }
                    // Assign payload_str.front() to newString.
                    payload_str.front() = newString;
                // Close the current scope block.
                }
                // Assign pmt::pmt_t frame_len to pmt::from_long(2 * payload_str.front().length()).
                pmt::pmt_t frame_len = pmt::from_long(2 * payload_str.front().length());
                // Call add_item_tag with arguments (0, nitems_written(0), pmt::string_to_symbol("frame_len"), frame_len).
                add_item_tag(0, nitems_written(0), pmt::string_to_symbol("frame_len"), frame_len);

                // Call add_item_tag with arguments (0, nitems_written(0), pmt::string_to_symbol("payload_str"), pmt::string_to_symbol(payload_str.front())).
                add_item_tag(0, nitems_written(0), pmt::string_to_symbol("payload_str"), pmt::string_to_symbol(payload_str.front()));

                // Call std::copy with arguments (payload_str.front().begin(), payload_str.front().end(), std::back_inserter(m_payload)).
                std::copy(payload_str.front().begin(), payload_str.front().end(), std::back_inserter(m_payload));

                // Iterate with loop parameters (unsigned int i = 0; i < m_payload.size(); i++).
                for (unsigned int i = 0; i < m_payload.size(); i++)
                // Open a new scope block.
                {
                    // Assign out[2 * i] to (m_payload[i] ^ whitening_seq[i]) & 0x0F.
                    out[2 * i] = (m_payload[i] ^ whitening_seq[i]) & 0x0F;
                    // Assign out[2 * i + 1] to (m_payload[i] ^ whitening_seq[i]) >> 4.
                    out[2 * i + 1] = (m_payload[i] ^ whitening_seq[i]) >> 4;
                // Close the current scope block.
                }
                // Assign noutput_items to 2 * m_payload.size().
                noutput_items = 2 * m_payload.size();
                // Clear all contents from m_payload.
                m_payload.clear();
                // Remove range payload_str.begin() from payload_str.
                payload_str.erase(payload_str.begin());
            // Close the current scope block.
            }
            // Handle the alternative branch when previous conditions fail.
            else
                // Assign noutput_items to 0.
                noutput_items = 0;
            // Return noutput_items to the caller.
            return noutput_items;
        // Close the current scope block.
        }

    // Close the current scope and emit the trailing comment.
    } /* namespace lora */
// Close the current scope and emit the trailing comment.
} /* namespace gr */
