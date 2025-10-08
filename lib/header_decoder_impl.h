// Compile the following section when INCLUDED_LORA_HEADER_DECODER_IMPL_H is not defined. (inline comment notes: Prevent multiple inclusion of the header decoder implementation declaration.)
#ifndef INCLUDED_LORA_HEADER_DECODER_IMPL_H  // Prevent multiple inclusion of the header decoder implementation declaration.
// Define macro INCLUDED_LORA_HEADER_DECODER_IMPL_H. (inline comment notes: Define the guard macro so repeated includes skip the body.)
#define INCLUDED_LORA_HEADER_DECODER_IMPL_H  // Define the guard macro so repeated includes skip the body.

// Bring in declarations from <gnuradio/lora_sdr/header_decoder.h>. (inline comment notes: Bring in the public header decoder interface definition.)
#include <gnuradio/lora_sdr/header_decoder.h>  // Bring in the public header decoder interface definition.
// Bring in declarations from <gnuradio/lora_sdr/utilities.h>. (inline comment notes: Include shared utility helpers used by the implementation.)
#include <gnuradio/lora_sdr/utilities.h>  // Include shared utility helpers used by the implementation.

// Start namespace gr scope. (inline comment notes: Begin the GNU Radio namespace scope.)
namespace gr {  // Begin the GNU Radio namespace scope.
  // Start namespace lora_sdr scope. (inline comment notes: Enter the LoRa SDR namespace where the module's blocks live.)
  namespace lora_sdr {  // Enter the LoRa SDR namespace where the module's blocks live.

    // Declare class header_decoder_impl. (inline comment notes: Declare the concrete implementation class derived from the public interface.)
    class header_decoder_impl : public header_decoder  // Declare the concrete implementation class derived from the public interface.
    // Open a new scope block.
    {
     // Begin private section of the class declaration.
     private:
        // Declare constant const uint8_t header_len. (inline comment notes: /< size of the header in nibbles  // Constant representing the fixed LoRa PHY header length in nibbles.)
        const uint8_t header_len = 5; ///< size of the header in nibbles  // Constant representing the fixed LoRa PHY header length in nibbles.

        // Declare bool m_impl_header. (inline comment notes: /< Specify if we use an explicit or implicit header  // Flag indicating whether incoming frames rely on implicit header information.)
        bool m_impl_header;///< Specify if we use an explicit or implicit header  // Flag indicating whether incoming frames rely on implicit header information.
        // Declare bool m_print_header. (inline comment notes: /< print or not header information in terminal  // Flag enabling verbose printing of decoded headers for debugging.)
        bool m_print_header; ///< print or not header information in terminal  // Flag enabling verbose printing of decoded headers for debugging.
        // Declare uint8_t m_payload_len. (inline comment notes: /< The payload length in bytes  // Stores the payload length for implicit header operation.)
        uint8_t m_payload_len;///< The payload length in bytes  // Stores the payload length for implicit header operation.
        // Declare bool m_has_crc. (inline comment notes: /< Specify the usage of a payload CRC  // Indicates whether a CRC is expected for the payload.)
        bool m_has_crc;///< Specify the usage of a payload CRC  // Indicates whether a CRC is expected for the payload.
        // Declare uint8_t m_cr. (inline comment notes: /< Coding rate  // Keeps track of the coding rate extracted from the header or configuration.)
        uint8_t m_cr;///< Coding rate  // Keeps track of the coding rate extracted from the header or configuration.
        // Declare uint8_t m_ldro_mode. (inline comment notes: /< use low datarate optimisation  // Stores the low data rate optimization mode provided or decoded.)
        uint8_t m_ldro_mode; ///< use low datarate optimisation  // Stores the low data rate optimization mode provided or decoded.

        // Declare uint8_t header_chk. (inline comment notes: /< The header checksum received in the header  // Holds the checksum nibble extracted from the header.)
        uint8_t header_chk; ///< The header checksum received in the header  // Holds the checksum nibble extracted from the header.

        // Declare uint32_t pay_cnt. (inline comment notes: /< The number of payload nibbles received  // Counts payload nibbles processed for the current frame.)
        uint32_t pay_cnt;///< The number of payload nibbles received  // Counts payload nibbles processed for the current frame.
        // Declare uint32_t nout. (inline comment notes: /< The number of data nibbles to output  // Tracks how many nibbles should be forwarded downstream.)
        uint32_t nout;///< The number of data nibbles to output  // Tracks how many nibbles should be forwarded downstream.
        // Declare bool is_header. (inline comment notes: /< Indicate that we need to decode the header  // Flag showing whether the block is currently parsing header symbols.)
        bool is_header ;///< Indicate that we need to decode the header  // Flag showing whether the block is currently parsing header symbols.

        /**
         *  \brief  Reset the block variables for a new frame.  // Declares a helper that reinitializes state between frames.
         */
        // Execute statement void new_frame_handler().
        void new_frame_handler();
        /**
         *  \brief publish decoding information contained in the header or provided to the block  // Declares the helper that publishes metadata about the decoded header.
         */
        // Execute statement void publish_frame_info(int cr, int pay_len, int crc, uint8_t ldro, int err).
        void publish_frame_info(int cr, int pay_len, int crc, uint8_t ldro, int err);

     // Begin public section of the class declaration.
     public:
      // Call header_decoder_impl with arguments (bool impl_head, uint8_t cr, uint32_t pay_len, bool has_crc, uint8_t ldro_mode, bool print_header). (inline comment notes: Constructor accepting runtime configuration for header decoding.)
      header_decoder_impl(bool impl_head, uint8_t cr, uint32_t pay_len, bool has_crc, uint8_t ldro_mode, bool print_header);  // Constructor accepting runtime configuration for header decoding.
      // Call ~header_decoder_impl with arguments (). (inline comment notes: Destructor declaration for the implementation class.)
      ~header_decoder_impl();  // Destructor declaration for the implementation class.

      // Execute statement void forecast (int noutput_items, gr_vector_int &ninput_items_required). (inline comment notes: Forecast override describing input requirements.)
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);  // Forecast override describing input requirements.

      // Specify parameter or initializer int general_work(int noutput_items. (inline comment notes: Primary work routine override handling header decoding and payload forwarding.)
      int general_work(int noutput_items,  // Primary work routine override handling header decoding and payload forwarding.
           // Specify parameter or initializer gr_vector_int &ninput_items. (inline comment notes: Gives access to the scheduler-provided counts of available input items.)
           gr_vector_int &ninput_items,  // Gives access to the scheduler-provided counts of available input items.
           // Specify parameter or initializer gr_vector_const_void_star &input_items. (inline comment notes: Provides const pointers to the incoming data buffers.)
           gr_vector_const_void_star &input_items,  // Provides const pointers to the incoming data buffers.
           // Declare gr_vector_void_star &output_items). (inline comment notes: Provides writable pointers to the outgoing data buffers.)
           gr_vector_void_star &output_items);  // Provides writable pointers to the outgoing data buffers.
    // Close the current scope and emit the trailing comment.
    };
  // Close the current scope block. (inline comment notes: namespace lora  // Close the LoRa SDR namespace scope.)
  } // namespace lora  // Close the LoRa SDR namespace scope.
// Close the current scope block. (inline comment notes: namespace gr  // Close the GNU Radio namespace scope.)
} // namespace gr  // Close the GNU Radio namespace scope.

// Close the preceding conditional compilation block. (inline comment notes: Terminate the include guard for this header file.)
#endif /* INCLUDED_LORA_HEADER_DECODER_IMPL_H */  // Terminate the include guard for this header file.
