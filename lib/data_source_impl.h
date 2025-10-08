// Compile the following section when INCLUDED_LORA_SDR_DATA_SOURCE_IMPL_H is not defined.
#ifndef INCLUDED_LORA_SDR_DATA_SOURCE_IMPL_H
// Define macro INCLUDED_LORA_SDR_DATA_SOURCE_IMPL_H.
#define INCLUDED_LORA_SDR_DATA_SOURCE_IMPL_H

// Bring in declarations from <gnuradio/lora_sdr/data_source.h>.
#include <gnuradio/lora_sdr/data_source.h>
// Bring in declarations from <gnuradio/lora_sdr/utilities.h>.
#include <gnuradio/lora_sdr/utilities.h>

// Start namespace gr scope.
namespace gr {
  // Start namespace lora_sdr scope.
  namespace lora_sdr {

    // Declare class data_source_impl.
    class data_source_impl : public data_source
    // Open a new scope block.
    {
     // Begin private section of the class declaration.
     private:
         // Declare int frame_cnt. (inline comment notes: /< count the number of frame sent)
         int frame_cnt; ///< count the number of frame sent
         // Declare int m_n_frames. (inline comment notes: /< The maximal number of frame to send)
         int m_n_frames;///< The maximal number of frame to send
         // Declare int m_pay_len. (inline comment notes: /< The payload length)
         int m_pay_len; ///< The payload length

         /**
          *  \brief  return a random string containing [a-z A-Z 0-9]
          *
          *  \param  nbytes
          *          The number of char in the string
          */
         // Execute statement std::string random_string(int nbytes).
         std::string random_string(int nbytes);
         /**
          *  \brief  Handles trigger messages
          */
         // Execute statement void trigg_handler(pmt::pmt_t id).
         void trigg_handler(pmt::pmt_t id);

     // Begin public section of the class declaration.
     public:
      // Call data_source_impl with arguments (int pay_len,int n_frames).
      data_source_impl(int pay_len,int n_frames);
      // Call ~data_source_impl with arguments ().
      ~data_source_impl();

      // Specify parameter or initializer int work(int noutput_items.
      int work(int noutput_items,
         // Specify parameter or initializer gr_vector_const_void_star &input_items.
         gr_vector_const_void_star &input_items,
         // Declare gr_vector_void_star &output_items).
         gr_vector_void_star &output_items);
    // Close the current scope and emit the trailing comment.
    };
  // Close the current scope block. (inline comment notes: namespace lora_sdr)
  } // namespace lora_sdr
// Close the current scope block. (inline comment notes: namespace gr)
} // namespace gr

// Close the preceding conditional compilation block.
#endif /* INCLUDED_LORA_SDR_DATA_SOURCE_IMPL_H */
