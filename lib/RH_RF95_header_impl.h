// Compile the following section when INCLUDED_LORA_SDR_RH_RF95_HEADER_IMPL_H is not defined.
#ifndef INCLUDED_LORA_SDR_RH_RF95_HEADER_IMPL_H
// Define macro INCLUDED_LORA_SDR_RH_RF95_HEADER_IMPL_H.
#define INCLUDED_LORA_SDR_RH_RF95_HEADER_IMPL_H

// Bring in declarations from <gnuradio/lora_sdr/RH_RF95_header.h>.
#include <gnuradio/lora_sdr/RH_RF95_header.h>

// Start namespace gr scope.
namespace gr {
  // Start namespace lora_sdr scope.
  namespace lora_sdr {

    // Declare class RH_RF95_header_impl.
    class RH_RF95_header_impl : public RH_RF95_header
    // Open a new scope block.
    {
     // Begin private section of the class declaration.
     private:
      // Declare char m_to. (inline comment notes: /< radiohead specific header field "to")
      char m_to;    ///< radiohead specific header field "to"
      // Declare char m_from. (inline comment notes: /< radiohead specific header field "from")
      char m_from;  ///< radiohead specific header field "from"
      // Declare char m_id. (inline comment notes: /< radiohead specific header field "id")
      char m_id;    ///< radiohead specific header field "id"
      // Declare char m_flags. (inline comment notes: /< radiohead specific header field "flags")
      char m_flags; ///< radiohead specific header field "flags"
      // Declare std::vector<uint8_t> m_payload. (inline comment notes: /<payload bytes)
      std::vector<uint8_t> m_payload; ///<payload bytes
      // Execute statement void msg_handler(pmt::pmt_t message).
      void msg_handler(pmt::pmt_t message);


     // Begin public section of the class declaration.
     public:
      // Call RH_RF95_header_impl with arguments (uint8_t _to, uint8_t _from, uint8_t _id, uint8_t _flags).
      RH_RF95_header_impl(uint8_t _to, uint8_t _from, uint8_t _id, uint8_t _flags);
      // Call ~RH_RF95_header_impl with arguments ().
      ~RH_RF95_header_impl();

      // Where all the action really happens
      // Execute statement void forecast (int noutput_items, gr_vector_int &ninput_items_required).
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      // Specify parameter or initializer int general_work(int noutput_items.
      int general_work(int noutput_items,
           // Specify parameter or initializer gr_vector_int &ninput_items.
           gr_vector_int &ninput_items,
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
#endif /* INCLUDED_LORA_SDR_RH_RF95_HEADER_IMPL_H */
