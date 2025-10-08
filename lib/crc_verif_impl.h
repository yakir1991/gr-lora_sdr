// Compile the following section when INCLUDED_LORA_CRC_VERIF_IMPL_H is not defined.
#ifndef INCLUDED_LORA_CRC_VERIF_IMPL_H
// Define macro INCLUDED_LORA_CRC_VERIF_IMPL_H.
#define INCLUDED_LORA_CRC_VERIF_IMPL_H

// Bring in declarations from <gnuradio/lora_sdr/crc_verif.h>.
#include <gnuradio/lora_sdr/crc_verif.h>

// #define GRLORA_DEBUG

// Start namespace gr scope.
namespace gr {
  // Start namespace lora_sdr scope.
  namespace lora_sdr {

    // Declare class crc_verif_impl.
    class crc_verif_impl : public crc_verif
    // Open a new scope block.
    {
     // Begin private section of the class declaration.
     private:
        // Declare uint32_t m_payload_len. (inline comment notes: /< Payload length in bytes)
        uint32_t m_payload_len;///< Payload length in bytes
        // Declare bool m_crc_presence. (inline comment notes: /< Indicate if there is a payload CRC)
        bool m_crc_presence;///< Indicate if there is a payload CRC
        // Declare uint16_t m_crc. (inline comment notes: /< The CRC calculated from the received payload)
        uint16_t m_crc;///< The CRC calculated from the received payload
        // Declare std::string message_str. (inline comment notes: /< The payload string)
        std::string message_str;///< The payload string
        // Declare char m_char. (inline comment notes: /< A new char of the payload)
        char m_char;///< A new char of the payload
        // Declare bool new_frame. (inline comment notes: /<indicate a new frame)
        bool new_frame; ///<indicate a new frame
        // Declare std::vector<uint8_t> in_buff. (inline comment notes: /< input buffer containing the data bytes and CRC if any)
        std::vector<uint8_t> in_buff;///< input buffer containing the data bytes and CRC if any
        // Declare int print_rx_msg. (inline comment notes: /< print received message in terminal. 0: no print, 1: ASCII, 2: HEX)
        int print_rx_msg;  ///< print received message in terminal. 0: no print, 1: ASCII, 2: HEX
        // Declare bool output_crc_check. (inline comment notes: /< output the result of the payload CRC check)
        bool output_crc_check; ///< output the result of the payload CRC check
        // Declare tag_t current_tag. (inline comment notes: /< the most recent tag for the packet we are currently processing)
        tag_t current_tag; ///< the most recent tag for the packet we are currently processing
        

        // Assign uint32_t cnt to 0. (inline comment notes: /< count the number of frame)
        uint32_t cnt=0;///< count the number of frame

        /**
         *  \brief  Handles the payload length received from the header_decoder block.
         */
        // Execute statement void header_pay_len_handler(pmt::pmt_t payload_len).
        void header_pay_len_handler(pmt::pmt_t payload_len);
        /**
         *  \brief  Handles the crc_presence received from the header_decoder block.
         */
        // Execute statement void header_crc_handler(pmt::pmt_t crc_presence).
        void header_crc_handler(pmt::pmt_t crc_presence);
        /**
         *  \brief  Calculate the CRC 16 using poly=0x1021 and Init=0x0000
         *
         *  \param  data
         *          The pointer to the data beginning.
         *  \param  len
         *          The length of the data in bytes.
         */
        // Execute statement unsigned int crc16(uint8_t* data, uint32_t len).
        unsigned int crc16(uint8_t* data, uint32_t len);

     // Begin public section of the class declaration.
     public:
      // Call crc_verif_impl with arguments (int print_rx_msg, bool output_crc_check).
      crc_verif_impl(int print_rx_msg, bool output_crc_check);
      // Call ~crc_verif_impl with arguments ().
      ~crc_verif_impl();

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
#endif /* INCLUDED_LORA_CRC_VERIF_IMPL_H */
