// Only compile the following section when HAVE_CONFIG_H is defined.
#ifdef HAVE_CONFIG_H
// Bring in declarations from "config.h".
#include "config.h"
// Close the preceding conditional compilation block.
#endif

// Bring in declarations from <gnuradio/io_signature.h>.
#include <gnuradio/io_signature.h>
// Bring in declarations from "hamming_enc_impl.h".
#include "hamming_enc_impl.h"
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

    // Specify the shared pointer typedef associated with hamming_enc.
    hamming_enc::sptr
    // Define the c::make function.
    hamming_enc::make(uint8_t cr, uint8_t sf)
    // Open a new scope block.
    {
      // Return gnuradio::get_initial_sptr(new hamming_enc_impl(cr, sf)) to the caller.
      return gnuradio::get_initial_sptr(new hamming_enc_impl(cr, sf));
    // Close the current scope block.
    }

    /*
     * The private constructor
     */
    // Define the l::hamming_enc_impl function.
    hamming_enc_impl::hamming_enc_impl(uint8_t cr, uint8_t sf)
        // Initialize base classes or members with gr::sync_block("hamming_enc",.
        : gr::sync_block("hamming_enc",
                         // Specify parameter or initializer gr::io_signature::make(1, 1, sizeof(uint8_t)).
                         gr::io_signature::make(1, 1, sizeof(uint8_t)),
                         // Define the e::make function.
                         gr::io_signature::make(1, 1, sizeof(uint8_t)))
    // Open a new scope block.
    {
      // Assign m_cr to cr.
      m_cr = cr;
      // Assign m_sf to sf.
      m_sf = sf;
      // Call set_tag_propagation_policy with arguments (TPP_ONE_TO_ONE).
      set_tag_propagation_policy(TPP_ONE_TO_ONE);
    // Close the current scope block.
    }


        // Define the l::set_cr function.
        void hamming_enc_impl::set_cr(uint8_t cr){
            // Assign m_cr to cr.
            m_cr = cr;
        // Close the current scope block.
        } 

      // Define the l::set_sf function.
      void hamming_enc_impl::set_sf(uint8_t sf){
          // Assign m_sf to sf.
          m_sf = sf;
      // Close the current scope block.
      } 

        // Define the l::get_cr function.
        uint8_t hamming_enc_impl::get_cr(){
            // Return m_cr to the caller.
            return m_cr;
        // Close the current scope block.
        } 


    /*
     * Our virtual destructor.
     */
    // Define the l::~hamming_enc_impl function.
    hamming_enc_impl::~hamming_enc_impl()
    // Open a new scope block.
    {
    // Close the current scope block.
    }

    // Execute int.
    int
    // Specify parameter or initializer hamming_enc_impl::work(int noutput_items.
    hamming_enc_impl::work(int noutput_items,
                           // Specify parameter or initializer gr_vector_const_void_star &input_items.
                           gr_vector_const_void_star &input_items,
                           // Execute gr_vector_void_star &output_items).
                           gr_vector_void_star &output_items)
    // Open a new scope block.
    {
      // Assign const uint8_t *in_data to (const uint8_t *)input_items[0].
      const uint8_t *in_data = (const uint8_t *)input_items[0];
      // Assign uint8_t *out to (uint8_t *)output_items[0].
      uint8_t *out = (uint8_t *)output_items[0];

      // Assign int nitems_to_process to noutput_items.
      int nitems_to_process = noutput_items;

      // read tags
      // Declare std::vector<tag_t> tags.
      std::vector<tag_t> tags;
      // Call get_tags_in_window with arguments (tags, 0, 0, noutput_items, pmt::string_to_symbol("configuration")).
      get_tags_in_window(tags, 0, 0, noutput_items, pmt::string_to_symbol("configuration"));

      // Branch when condition (tags.size() > 0) evaluates to true.
      if (tags.size() > 0) {
          // Update cr and sf
          // Assign pmt::pmt_t err to pmt::string_to_symbol("error").
          pmt::pmt_t err = pmt::string_to_symbol("error");
          // Assign int new_cr to pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("cr"), err)).
          int new_cr = pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("cr"), err));
          // Assign int new_sf to pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("sf"), err)).
          int new_sf = pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("sf"), err));
          // Branch when condition (new_cr != m_cr) evaluates to true.
          if (new_cr != m_cr) {
              // Assign m_cr to new_cr.
              m_cr = new_cr;
              // std::cout<<"New cr Hamming "<< static_cast<int>(m_cr) <<std::endl;
          // Close the current scope block.
          }
          // Branch when condition (new_sf != m_sf) evaluates to true.
          if (new_sf != m_sf) {
              // Assign m_sf to new_sf.
              m_sf = new_sf;
              // std::cout<<"New sf Hamming "<< static_cast<int>(m_sf) <<std::endl;
          // Close the current scope block.
          }
      // Close the current scope block.
      }
      

      // Call get_tags_in_window with arguments (tags, 0, 0, noutput_items, pmt::string_to_symbol("frame_len")).
      get_tags_in_window(tags, 0, 0, noutput_items, pmt::string_to_symbol("frame_len"));
      // Branch when condition (tags.size()) evaluates to true.
      if (tags.size())
      // Open a new scope block.
      {
        // Branch when condition (tags[0].offset != nitems_read(0)) evaluates to true.
        if (tags[0].offset != nitems_read(0))
          // Assign nitems_to_process to tags[0].offset - nitems_read(0).
          nitems_to_process = tags[0].offset - nitems_read(0);
        // Handle the alternative branch when previous conditions fail.
        else
        // Open a new scope block.
        {
          // Branch when condition (tags.size() >= 2) evaluates to true.
          if (tags.size() >= 2)
            // Assign nitems_to_process to tags[1].offset - tags[0].offset.
            nitems_to_process = tags[1].offset - tags[0].offset;
          // Assign m_cnt to 0.
          m_cnt = 0;
        // Close the current scope block.
        }
      // Close the current scope block.
      }


      // Declare std::vector<bool> data_bin.
      std::vector<bool> data_bin;
      // Declare bool p0, p1, p2, p3, p4.
      bool p0, p1, p2, p3, p4;
      // Iterate with loop parameters (int i = 0; i < nitems_to_process; i++).
      for (int i = 0; i < nitems_to_process; i++)
      // Open a new scope block.
      {
// Only compile the following section when GRLORA_DEBUG is defined.
#ifdef GRLORA_DEBUG
        // Execute statement std::cout << std::hex << (int)in_data[i] << "   ".
        std::cout << std::hex << (int)in_data[i] << "   ";
// Close the preceding conditional compilation block.
#endif
        // Assign uint8_t cr_app to (m_cnt < m_sf - 2) ? 4 : m_cr.
        uint8_t cr_app = (m_cnt < m_sf - 2) ? 4 : m_cr;
        // Assign data_bin to int2bool(in_data[i], 4).
        data_bin = int2bool(in_data[i], 4);

        //the data_bin is msb first
        // Branch when condition (cr_app != 1) evaluates to true.
        if (cr_app != 1)
        // Open a new scope block. (inline comment notes: need hamming parity bits)
        { //need hamming parity bits
          // Assign p0 to data_bin[3] ^ data_bin[2] ^ data_bin[1].
          p0 = data_bin[3] ^ data_bin[2] ^ data_bin[1];
          // Assign p1 to data_bin[2] ^ data_bin[1] ^ data_bin[0].
          p1 = data_bin[2] ^ data_bin[1] ^ data_bin[0];
          // Assign p2 to data_bin[3] ^ data_bin[2] ^ data_bin[0].
          p2 = data_bin[3] ^ data_bin[2] ^ data_bin[0];
          // Assign p3 to data_bin[3] ^ data_bin[1] ^ data_bin[0].
          p3 = data_bin[3] ^ data_bin[1] ^ data_bin[0];
          //we put the data LSB first and append the parity bits
          // Assign out[i] to (data_bin[3] << 7 | data_bin[2] << 6 | data_bin[1] << 5 | data_bin[0] << 4 | p0 << 3 | p1 << 2 | p2 << 1 | p3) >> (4 - cr_app).
          out[i] = (data_bin[3] << 7 | data_bin[2] << 6 | data_bin[1] << 5 | data_bin[0] << 4 | p0 << 3 | p1 << 2 | p2 << 1 | p3) >> (4 - cr_app);
        // Close the current scope block.
        }
        // Handle the alternative branch when previous conditions fail.
        else
        // Open a new scope block. (inline comment notes: coding rate = 4/5 we add a parity bit)
        { // coding rate = 4/5 we add a parity bit
          // Assign p4 to data_bin[0] ^ data_bin[1] ^ data_bin[2] ^ data_bin[3].
          p4 = data_bin[0] ^ data_bin[1] ^ data_bin[2] ^ data_bin[3];
          // Assign out[i] to (data_bin[3] << 4 | data_bin[2] << 3 | data_bin[1] << 2 | data_bin[0] << 1 | p4).
          out[i] = (data_bin[3] << 4 | data_bin[2] << 3 | data_bin[1] << 2 | data_bin[0] << 1 | p4);
        // Close the current scope block.
        }
// Only compile the following section when GRLORA_DEBUG is defined.
#ifdef GRLORA_DEBUG
        // Execute statement std::cout << std::hex << (int)out[i] << std::dec << std::endl.
        std::cout << std::hex << (int)out[i] << std::dec << std::endl;
// Close the preceding conditional compilation block.
#endif
        // Increment m_cnt.
        m_cnt++;
      // Close the current scope block.
      }

      // Return nitems_to_process to the caller.
      return nitems_to_process;
    // Close the current scope block.
    }

  // Close the current scope and emit the trailing comment.
  } /* namespace lora */
// Close the current scope and emit the trailing comment.
} /* namespace gr */
