// Compile the following section when INCLUDED_LORA_ADD_CRC_IMPL_H is not defined.
#ifndef INCLUDED_LORA_ADD_CRC_IMPL_H
// Define macro INCLUDED_LORA_ADD_CRC_IMPL_H.
#define INCLUDED_LORA_ADD_CRC_IMPL_H


// Bring in declarations from <gnuradio/lora_sdr/add_crc.h>.
#include <gnuradio/lora_sdr/add_crc.h>
// Bring in declarations from <gnuradio/lora_sdr/utilities.h>.
#include <gnuradio/lora_sdr/utilities.h>
// Start namespace gr scope.
namespace gr {
  // Start namespace lora_sdr scope.
  namespace lora_sdr {

    // Declare class add_crc_impl.
    class add_crc_impl : public add_crc
    // Open a new scope block.
    {
     // Begin private section of the class declaration.
     private:
        // Declare bool m_has_crc. (inline comment notes: /<indicate the presence of a payload CRC)
        bool m_has_crc; ///<indicate the presence of a payload CRC
        // Declare std::vector<uint8_t> m_payload. (inline comment notes: /< payload data)
        std::vector<uint8_t> m_payload; ///< payload data
        // Declare uint8_t m_payload_len. (inline comment notes: /< length of the payload in Bytes)
        uint8_t m_payload_len; ///< length of the payload in Bytes
        // Declare int m_frame_len. (inline comment notes: /< length of the frame in number of gnuradio items)
        int m_frame_len; ///< length of the frame in number of gnuradio items
        // Declare int m_cnt. (inline comment notes: /< counter of the number of symbol in frame)
        int m_cnt; ///< counter of the number of symbol in frame

        // Execute statement unsigned int crc16(unsigned int crcValue, unsigned char newByte).
        unsigned int crc16(unsigned int crcValue, unsigned char newByte);

     // Begin public section of the class declaration.
     public:
      // Call add_crc_impl with arguments (bool has_crc).
      add_crc_impl(bool has_crc);
      // Call ~add_crc_impl with arguments ().
      ~add_crc_impl();

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
  // Close the current scope block. (inline comment notes: namespace lora)
  } // namespace lora
// Close the current scope block. (inline comment notes: namespace gr)
} // namespace gr

// Close the preceding conditional compilation block.
#endif /* INCLUDED_LORA_ADD_CRC_IMPL_H */
