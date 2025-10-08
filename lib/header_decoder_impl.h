#ifndef INCLUDED_LORA_HEADER_DECODER_IMPL_H  // Prevent multiple inclusion of the header decoder implementation declaration.
#define INCLUDED_LORA_HEADER_DECODER_IMPL_H  // Define the guard macro so repeated includes skip the body.

#include <gnuradio/lora_sdr/header_decoder.h>  // Bring in the public header decoder interface definition.
#include <gnuradio/lora_sdr/utilities.h>  // Include shared utility helpers used by the implementation.

namespace gr {  // Begin the GNU Radio namespace scope.
  namespace lora_sdr {  // Enter the LoRa SDR namespace where the module's blocks live.

    class header_decoder_impl : public header_decoder  // Declare the concrete implementation class derived from the public interface.
    {
     private:
        const uint8_t header_len = 5; ///< size of the header in nibbles  // Constant representing the fixed LoRa PHY header length in nibbles.

        bool m_impl_header;///< Specify if we use an explicit or implicit header  // Flag indicating whether incoming frames rely on implicit header information.
        bool m_print_header; ///< print or not header information in terminal  // Flag enabling verbose printing of decoded headers for debugging.
        uint8_t m_payload_len;///< The payload length in bytes  // Stores the payload length for implicit header operation.
        bool m_has_crc;///< Specify the usage of a payload CRC  // Indicates whether a CRC is expected for the payload.
        uint8_t m_cr;///< Coding rate  // Keeps track of the coding rate extracted from the header or configuration.
        uint8_t m_ldro_mode; ///< use low datarate optimisation  // Stores the low data rate optimization mode provided or decoded.

        uint8_t header_chk; ///< The header checksum received in the header  // Holds the checksum nibble extracted from the header.

        uint32_t pay_cnt;///< The number of payload nibbles received  // Counts payload nibbles processed for the current frame.
        uint32_t nout;///< The number of data nibbles to output  // Tracks how many nibbles should be forwarded downstream.
        bool is_header ;///< Indicate that we need to decode the header  // Flag showing whether the block is currently parsing header symbols.

        /**
         *  \brief  Reset the block variables for a new frame.  // Declares a helper that reinitializes state between frames.
         */
        void new_frame_handler();
        /**
         *  \brief publish decoding information contained in the header or provided to the block  // Declares the helper that publishes metadata about the decoded header.
         */
        void publish_frame_info(int cr, int pay_len, int crc, uint8_t ldro, int err);

     public:
      header_decoder_impl(bool impl_head, uint8_t cr, uint32_t pay_len, bool has_crc, uint8_t ldro_mode, bool print_header);  // Constructor accepting runtime configuration for header decoding.
      ~header_decoder_impl();  // Destructor declaration for the implementation class.

      void forecast (int noutput_items, gr_vector_int &ninput_items_required);  // Forecast override describing input requirements.

      int general_work(int noutput_items,  // Primary work routine override handling header decoding and payload forwarding.
           gr_vector_int &ninput_items,  // Gives access to the scheduler-provided counts of available input items.
           gr_vector_const_void_star &input_items,  // Provides const pointers to the incoming data buffers.
           gr_vector_void_star &output_items);  // Provides writable pointers to the outgoing data buffers.
    };
  } // namespace lora  // Close the LoRa SDR namespace scope.
} // namespace gr  // Close the GNU Radio namespace scope.

#endif /* INCLUDED_LORA_HEADER_DECODER_IMPL_H */  // Terminate the include guard for this header file.
