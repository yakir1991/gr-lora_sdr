#ifndef INCLUDED_LORA_HEADER_IMPL_H  // Prevent multiple inclusions of the header implementation definition.
#define INCLUDED_LORA_HEADER_IMPL_H  // Define the include guard symbol for subsequent conditional checks.

#include <gnuradio/lora_sdr/header.h>  // Pull in the public header interface that this implementation fulfills.

namespace gr {  // Start the GNU Radio namespace to scope all block classes.
  namespace lora_sdr {  // Enter the LoRa SDR namespace where custom LoRa blocks live.

    class header_impl : public header  // Declare the concrete header implementation derived from the public interface.
    {
     private:
      bool m_impl_head; ///< indicate if the header is implicit  // Flag indicating whether implicit header mode is selected.
      bool m_has_crc; ///< indicate the presence of a payload crc  // Flag showing if the payload includes a CRC field.
      uint8_t m_cr; ///< Transmission coding rate  // Stores the LoRa coding rate used for parity generation.
      uint8_t m_payload_len; ///< Payload length  // Keeps track of the payload length extracted from input tags.
      unsigned int m_cnt_nibbles; ///< count the processes nibbles in a frame  // Counts how many nibbles have been forwarded for the current frame.
      unsigned int m_cnt_header_nibbles; ///< count the number of explicit header nibbles output  // Counts how many header nibbles were produced so far.
      std::vector<uint8_t> m_header; ///< contain the header to prepend  // Buffer storing the generated header nibbles for explicit mode.

      bool m_has_config_tag; ///<indicate that a configuration tag was received  // Flag noting whether a configuration tag was captured for forwarding.


      std::vector<tag_t> m_tags;  // Temporary storage for the tags that must be re-emitted on the output stream.
      void msg_handler(pmt::pmt_t message);  // Declare the message handler used when the block receives asynchronous PMT messages.

     public:
      header_impl(bool impl_head, bool has_crc, uint8_t cr);  // Construct the implementation specifying header mode, CRC presence, and coding rate.
      ~header_impl();  // Declare the destructor allowing clean shutdown of the block.

      void set_cr(uint8_t cr);  // Allow external code to change the coding rate at runtime.
      uint8_t get_cr();  // Provide a getter returning the currently stored coding rate value.


      void forecast (int noutput_items, gr_vector_int &ninput_items_required);  // Override forecast to describe input requirements for scheduling.

      int general_work(int noutput_items,  // Override general_work to process incoming nibbles and insert header data.
           gr_vector_int &ninput_items,  // Report available input items through the scheduler-provided vector.
           gr_vector_const_void_star &input_items,  // Access the immutable input buffers to read from the stream.
           gr_vector_void_star &output_items);  // Access the mutable output buffers to write produced symbols.

    };

  } // namespace lora  // Close the lora_sdr namespace scope.
} // namespace gr  // Close the GNU Radio namespace scope.

#endif /* INCLUDED_LORA_HEADER_IMPL_H */  // End the include guard definition for this header.
