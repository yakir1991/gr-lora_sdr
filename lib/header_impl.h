// Compile the following section when INCLUDED_LORA_HEADER_IMPL_H is not defined. (inline comment notes: Prevent multiple inclusions of the header implementation definition.)
#ifndef INCLUDED_LORA_HEADER_IMPL_H  // Prevent multiple inclusions of the header implementation definition.
// Define macro INCLUDED_LORA_HEADER_IMPL_H. (inline comment notes: Define the include guard symbol for subsequent conditional checks.)
#define INCLUDED_LORA_HEADER_IMPL_H  // Define the include guard symbol for subsequent conditional checks.

// Bring in declarations from <gnuradio/lora_sdr/header.h>. (inline comment notes: Pull in the public header interface that this implementation fulfills.)
#include <gnuradio/lora_sdr/header.h>  // Pull in the public header interface that this implementation fulfills.

// Start namespace gr scope. (inline comment notes: Start the GNU Radio namespace to scope all block classes.)
namespace gr {  // Start the GNU Radio namespace to scope all block classes.
  // Start namespace lora_sdr scope. (inline comment notes: Enter the LoRa SDR namespace where custom LoRa blocks live.)
  namespace lora_sdr {  // Enter the LoRa SDR namespace where custom LoRa blocks live.

    // Declare class header_impl. (inline comment notes: Declare the concrete header implementation derived from the public interface.)
    class header_impl : public header  // Declare the concrete header implementation derived from the public interface.
    // Open a new scope block.
    {
     // Begin private section of the class declaration.
     private:
      // Declare bool m_impl_head. (inline comment notes: /< indicate if the header is implicit  // Flag indicating whether implicit header mode is selected.)
      bool m_impl_head; ///< indicate if the header is implicit  // Flag indicating whether implicit header mode is selected.
      // Declare bool m_has_crc. (inline comment notes: /< indicate the presence of a payload crc  // Flag showing if the payload includes a CRC field.)
      bool m_has_crc; ///< indicate the presence of a payload crc  // Flag showing if the payload includes a CRC field.
      // Declare uint8_t m_cr. (inline comment notes: /< Transmission coding rate  // Stores the LoRa coding rate used for parity generation.)
      uint8_t m_cr; ///< Transmission coding rate  // Stores the LoRa coding rate used for parity generation.
      // Declare uint8_t m_payload_len. (inline comment notes: /< Payload length  // Keeps track of the payload length extracted from input tags.)
      uint8_t m_payload_len; ///< Payload length  // Keeps track of the payload length extracted from input tags.
      // Declare unsigned int m_cnt_nibbles. (inline comment notes: /< count the processes nibbles in a frame  // Counts how many nibbles have been forwarded for the current frame.)
      unsigned int m_cnt_nibbles; ///< count the processes nibbles in a frame  // Counts how many nibbles have been forwarded for the current frame.
      // Declare unsigned int m_cnt_header_nibbles. (inline comment notes: /< count the number of explicit header nibbles output  // Counts how many header nibbles were produced so far.)
      unsigned int m_cnt_header_nibbles; ///< count the number of explicit header nibbles output  // Counts how many header nibbles were produced so far.
      // Declare std::vector<uint8_t> m_header. (inline comment notes: /< contain the header to prepend  // Buffer storing the generated header nibbles for explicit mode.)
      std::vector<uint8_t> m_header; ///< contain the header to prepend  // Buffer storing the generated header nibbles for explicit mode.

      // Declare bool m_has_config_tag. (inline comment notes: /<indicate that a configuration tag was received  // Flag noting whether a configuration tag was captured for forwarding.)
      bool m_has_config_tag; ///<indicate that a configuration tag was received  // Flag noting whether a configuration tag was captured for forwarding.


      // Declare std::vector<tag_t> m_tags. (inline comment notes: Temporary storage for the tags that must be re-emitted on the output stream.)
      std::vector<tag_t> m_tags;  // Temporary storage for the tags that must be re-emitted on the output stream.
      // Execute statement void msg_handler(pmt::pmt_t message). (inline comment notes: Declare the message handler used when the block receives asynchronous PMT messages.)
      void msg_handler(pmt::pmt_t message);  // Declare the message handler used when the block receives asynchronous PMT messages.

     // Begin public section of the class declaration.
     public:
      // Call header_impl with arguments (bool impl_head, bool has_crc, uint8_t cr). (inline comment notes: Construct the implementation specifying header mode, CRC presence, and coding rate.)
      header_impl(bool impl_head, bool has_crc, uint8_t cr);  // Construct the implementation specifying header mode, CRC presence, and coding rate.
      // Call ~header_impl with arguments (). (inline comment notes: Declare the destructor allowing clean shutdown of the block.)
      ~header_impl();  // Declare the destructor allowing clean shutdown of the block.

      // Execute statement void set_cr(uint8_t cr). (inline comment notes: Allow external code to change the coding rate at runtime.)
      void set_cr(uint8_t cr);  // Allow external code to change the coding rate at runtime.
      // Execute statement uint8_t get_cr(). (inline comment notes: Provide a getter returning the currently stored coding rate value.)
      uint8_t get_cr();  // Provide a getter returning the currently stored coding rate value.


      // Execute statement void forecast (int noutput_items, gr_vector_int &ninput_items_required). (inline comment notes: Override forecast to describe input requirements for scheduling.)
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);  // Override forecast to describe input requirements for scheduling.

      // Specify parameter or initializer int general_work(int noutput_items. (inline comment notes: Override general_work to process incoming nibbles and insert header data.)
      int general_work(int noutput_items,  // Override general_work to process incoming nibbles and insert header data.
           // Specify parameter or initializer gr_vector_int &ninput_items. (inline comment notes: Report available input items through the scheduler-provided vector.)
           gr_vector_int &ninput_items,  // Report available input items through the scheduler-provided vector.
           // Specify parameter or initializer gr_vector_const_void_star &input_items. (inline comment notes: Access the immutable input buffers to read from the stream.)
           gr_vector_const_void_star &input_items,  // Access the immutable input buffers to read from the stream.
           // Declare gr_vector_void_star &output_items). (inline comment notes: Access the mutable output buffers to write produced symbols.)
           gr_vector_void_star &output_items);  // Access the mutable output buffers to write produced symbols.

    // Close the current scope and emit the trailing comment.
    };

  // Close the current scope block. (inline comment notes: namespace lora  // Close the lora_sdr namespace scope.)
  } // namespace lora  // Close the lora_sdr namespace scope.
// Close the current scope block. (inline comment notes: namespace gr  // Close the GNU Radio namespace scope.)
} // namespace gr  // Close the GNU Radio namespace scope.

// Close the preceding conditional compilation block. (inline comment notes: End the include guard definition for this header.)
#endif /* INCLUDED_LORA_HEADER_IMPL_H */  // End the include guard definition for this header.
