
// Only compile the following section when HAVE_CONFIG_H is defined.
#ifdef HAVE_CONFIG_H
// Bring in declarations from "config.h".
#include "config.h"
// Close the preceding conditional compilation block.
#endif

// Bring in declarations from <gnuradio/io_signature.h>.
#include <gnuradio/io_signature.h>
// Bring in declarations from "data_source_impl.h".
#include "data_source_impl.h"
// Bring in declarations from <gnuradio/block.h>.
#include <gnuradio/block.h>

// Start namespace gr scope.
namespace gr {
  // Start namespace lora_sdr scope.
  namespace lora_sdr {

    // Specify the shared pointer typedef associated with data_source.
    data_source::sptr
    // Define the e::make function.
    data_source::make(int pay_len,int n_frames)
    // Open a new scope block.
    {
      // Return gnuradio::get_initial_sptr to the caller.
      return gnuradio::get_initial_sptr
        // Construct a new object with expression (new data_source_impl(pay_len, n_frames));.
        (new data_source_impl(pay_len, n_frames));
    // Close the current scope block.
    }

    /*
     * The private constructor
     */
    // Define the l::data_source_impl function.
    data_source_impl::data_source_impl(int pay_len,int n_frames)
      // Initialize base classes or members with gr::sync_block("data_source",.
      : gr::sync_block("data_source",
              // Specify parameter or initializer gr::io_signature::make(0, 0, 0).
              gr::io_signature::make(0, 0, 0),
              // Define the e::make function.
              gr::io_signature::make(0, 0, 0))
    // Open a new scope block.
    {
        // Assign m_n_frames to n_frames.
        m_n_frames = n_frames;
        // Assign m_pay_len to pay_len.
        m_pay_len = pay_len;
        // Assign frame_cnt to -5. (inline comment notes: let some time to the Rx to start listening)
        frame_cnt = -5;// let some time to the Rx to start listening
        // Call message_port_register_in with arguments (pmt::mp("trigg")).
        message_port_register_in(pmt::mp("trigg"));
        // set_msg_handler(pmt::mp("trigg"),boost::bind(&data_source_impl::trigg_handler, this, _1));
        // Define the r function.
        set_msg_handler(pmt::mp("trigg"), [this](pmt::pmt_t msg) { this->trigg_handler(msg); });

        // Call message_port_register_out with arguments (pmt::mp("msg")).
        message_port_register_out(pmt::mp("msg"));
    // Close the current scope block.
    }

    /*
     * Our virtual destructor.
     */
    // Define the l::~data_source_impl function.
    data_source_impl::~data_source_impl()
    // Execute {}.
    {}
    // Define the l::random_string function.
    std::string data_source_impl::random_string(int Nbytes){
        // Declare constant const char* charmap.
        const char* charmap = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        // Assign const size_t charmapLength to strlen(charmap).
        const size_t charmapLength = strlen(charmap);
        // Declare auto-deduced variable auto generator.
        auto generator = [&](){ return charmap[rand()%charmapLength]; };
        // Declare std::string result.
        std::string result;
        // Execute statement result.reserve(Nbytes).
        result.reserve(Nbytes);
        // Call std::generate_n with arguments (std::back_inserter(result), Nbytes, generator).
        std::generate_n(std::back_inserter(result), Nbytes, generator);
        // Return result to the caller.
        return result;
    // Close the current scope block.
    }
    // Define the l::trigg_handler function.
    void data_source_impl::trigg_handler(pmt::pmt_t msg){
        // Branch when condition (frame_cnt<m_n_frames&&frame_cnt>=0) evaluates to true. (inline comment notes: send a new payload)
        if(frame_cnt<m_n_frames&&frame_cnt>=0){//send a new payload
            // Assign std::string str to random_string(m_pay_len).
            std::string str = random_string(m_pay_len);
            // Call message_port_pub with arguments (pmt::intern("msg"),pmt::mp(str)).
            message_port_pub(pmt::intern("msg"),pmt::mp(str));
            // Branch when condition (!mod(frame_cnt,50)) evaluates to true.
            if(!mod(frame_cnt,50))
                // Declare std::cout <<frame_cnt<< "/"<<m_n_frames <<std::endl.
                std::cout <<frame_cnt<< "/"<<m_n_frames <<std::endl;
            // Increment frame_cnt.
            frame_cnt++;
        // Close the current scope block.
        }
        // Define the f function. (inline comment notes: wait some time for Rx to start listening)
        else if(frame_cnt<m_n_frames)//wait some time for Rx to start listening
            // Increment frame_cnt.
            frame_cnt++;
        // Define the f function.
        else if(frame_cnt==m_n_frames){
            // Declare std::cout << "Done "<<m_n_frames<<" frames" << '\n'.
            std::cout << "Done "<<m_n_frames<<" frames" << '\n';
            // Increment frame_cnt.
            frame_cnt++;
        // Close the current scope block.
        }
    // Close the current scope block.
    }

    // Specify parameter or initializer int data_source_impl::work(int noutput_items.
    int data_source_impl::work(int noutput_items,
        // Specify parameter or initializer gr_vector_const_void_star &input_items.
        gr_vector_const_void_star &input_items,
        // Execute gr_vector_void_star &output_items).
        gr_vector_void_star &output_items)
    // Open a new scope block.
    {
      // Return 0 to the caller.
      return 0;
    // Close the current scope block.
    }

  // Close the current scope and emit the trailing comment.
  } /* namespace lora_sdr */
// Close the current scope and emit the trailing comment.
} /* namespace gr */
