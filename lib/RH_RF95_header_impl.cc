// Only compile the following section when HAVE_CONFIG_H is defined.
#ifdef HAVE_CONFIG_H
// Bring in declarations from "config.h".
#include "config.h"
// Close the preceding conditional compilation block.
#endif

// Bring in declarations from <gnuradio/io_signature.h>.
#include <gnuradio/io_signature.h>
// Bring in declarations from "RH_RF95_header_impl.h".
#include "RH_RF95_header_impl.h"

// Start namespace gr scope.
namespace gr {
  // Start namespace lora_sdr scope.
  namespace lora_sdr {

    // Specify the shared pointer typedef associated with RH_RF95_header.
    RH_RF95_header::sptr
    // Define the r::make function.
    RH_RF95_header::make(uint8_t _to, uint8_t _from, uint8_t _id, uint8_t _flags)
    // Open a new scope block.
    {
      // Return gnuradio::get_initial_sptr to the caller.
      return gnuradio::get_initial_sptr
        // Construct a new object with expression (new RH_RF95_header_impl(_to, _from, _id, _flags));.
        (new RH_RF95_header_impl(_to, _from, _id, _flags));
    // Close the current scope block.
    }

    /*
     * The private constructor
     */
    // Define the l::RH_RF95_header_impl function.
    RH_RF95_header_impl::RH_RF95_header_impl(uint8_t _to, uint8_t _from, uint8_t _id, uint8_t _flags)
      // Initialize base classes or members with gr::block("RH_RF95_header",.
      : gr::block("RH_RF95_header",
              // Specify parameter or initializer gr::io_signature::make(0, 0, 0).
              gr::io_signature::make(0, 0, 0),
              // Define the e::make function.
              gr::io_signature::make(0, 0, 0))
      // Open a new scope block.
      {
            // Assign m_to to _to.
            m_to = _to;
            // Assign m_from to _from.
            m_from = _from;
            // Assign m_id to _id.
            m_id = _id;
            // Assign m_flags to _flags.
            m_flags = _flags;

            // Call message_port_register_out with arguments (pmt::mp("msg")).
            message_port_register_out(pmt::mp("msg"));
            // Call message_port_register_in with arguments (pmt::mp("msg")).
            message_port_register_in(pmt::mp("msg"));
            // set_msg_handler(pmt::mp("msg"), boost::bind(&RH_RF95_header_impl::msg_handler, this, _1));
            // Define the r function.
            set_msg_handler(pmt::mp("msg"), [this](pmt::pmt_t msg) { this->msg_handler(msg); });
      // Close the current scope block.
      }
  /*
     * Our virtual destructor.
     */
    // Define the l::~RH_RF95_header_impl function.
    RH_RF95_header_impl::~RH_RF95_header_impl()
    // Execute {}.
    {}

    // Execute void.
    void
    // Define the l::forecast function.
    RH_RF95_header_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    // Execute {}.
    {}
    // Define the l::msg_handler function.
    void RH_RF95_header_impl::msg_handler(pmt::pmt_t message){
     // Assign std::string str to pmt::symbol_to_string(message).
     std::string str=pmt::symbol_to_string(message);
     // Execute statement std::string s({ m_to,m_from,m_id,m_flags }).
     std::string s({ m_to,m_from,m_id,m_flags });
     // Assign str to s+str.
     str=s+str;
     // Call message_port_pub with arguments (pmt::intern("msg"),pmt::mp(str)).
     message_port_pub(pmt::intern("msg"),pmt::mp(str));
  // Close the current scope block.
  }

    // Execute int.
    int
    // Specify parameter or initializer RH_RF95_header_impl::general_work (int noutput_items.
    RH_RF95_header_impl::general_work (int noutput_items,
                       // Specify parameter or initializer gr_vector_int &ninput_items.
                       gr_vector_int &ninput_items,
                       // Specify parameter or initializer gr_vector_const_void_star &input_items.
                       gr_vector_const_void_star &input_items,
                       // Execute gr_vector_void_star &output_items).
                       gr_vector_void_star &output_items)
    // Open a new scope block.
    {
      // Execute statement std::cout<<"there"<<std::endl.
      std::cout<<"there"<<std::endl;
      // Tell runtime system how many output items we produced.
      // Return 0 to the caller.
      return 0;
    // Close the current scope block.
    }
  // Close the current scope and emit the trailing comment.
  } /* namespace lora_sdr */
// Close the current scope and emit the trailing comment.
} /* namespace gr */
