// Compile the following section when INCLUDED_LORA_DEWHITENING_IMPL_H is not defined.
#ifndef INCLUDED_LORA_DEWHITENING_IMPL_H
// Define macro INCLUDED_LORA_DEWHITENING_IMPL_H.
#define INCLUDED_LORA_DEWHITENING_IMPL_H

// #define GRLORA_DEBUG
// Bring in declarations from <gnuradio/lora_sdr/dewhitening.h>.
#include <gnuradio/lora_sdr/dewhitening.h>

// Start namespace gr scope.
namespace gr {
  // Start namespace lora_sdr scope.
  namespace lora_sdr {

    // Declare class dewhitening_impl.
    class dewhitening_impl : public dewhitening
    // Open a new scope block.
    {
     // Begin private section of the class declaration.
     private:
        // Declare int m_payload_len. (inline comment notes: /< Payload length in bytes)
        int m_payload_len;  ///< Payload length in bytes
        // Declare int m_crc_presence. (inline comment notes: /< indicate the presence of a CRC)
        int m_crc_presence; ///< indicate the presence of a CRC
        // Assign int offset to 0. (inline comment notes: /< The offset in the whitening table)
        int offset = 0;       ///< The offset in the whitening table
        // Declare std::vector<uint8_t> dewhitened. (inline comment notes: /< The dewhitened bytes)
        std::vector<uint8_t> dewhitened; ///< The dewhitened bytes

        /**
         *  \brief  Handles the payload length received from the header_decoder block.
         */
        // Execute statement void header_pay_len_handler(pmt::pmt_t payload_len).
        void header_pay_len_handler(pmt::pmt_t payload_len);

        /**
         *  \brief  Reset the block variables for a new frame.
         */
        // Execute statement void new_frame_handler(pmt::pmt_t id).
        void new_frame_handler(pmt::pmt_t id);
        /**
         *  \brief  Receive indication on the CRC presence
         */
        // Execute statement void header_crc_handler(pmt::pmt_t crc_presence).
        void header_crc_handler(pmt::pmt_t crc_presence);

     // Begin public section of the class declaration.
     public:
      // Call dewhitening_impl with arguments ().
      dewhitening_impl();
      // Call ~dewhitening_impl with arguments ().
      ~dewhitening_impl();

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

  // Close the current scope block. (inline comment notes: namespace lora)
  } // namespace lora
// Close the current scope block. (inline comment notes: namespace gr)
} // namespace gr

// Close the preceding conditional compilation block.
#endif /* INCLUDED_LORA_DEWHITENING_IMPL_H */
