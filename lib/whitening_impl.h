

// Compile the following section when INCLUDED_LORA_WHITENING_IMPL_H is not defined.
#ifndef INCLUDED_LORA_WHITENING_IMPL_H
// Define macro INCLUDED_LORA_WHITENING_IMPL_H.
#define INCLUDED_LORA_WHITENING_IMPL_H

// Bring in declarations from <gnuradio/lora_sdr/whitening.h>.
#include <gnuradio/lora_sdr/whitening.h>
// Bring in declarations from <gnuradio/lora_sdr/utilities.h>.
#include <gnuradio/lora_sdr/utilities.h>
// Start namespace gr scope.
namespace gr
// Open a new scope block.
{
  // Start namespace lora_sdr scope.
  namespace lora_sdr
  // Open a new scope block.
  {

    // Declare class whitening_impl.
    class whitening_impl : public whitening
    // Open a new scope block.
    {
    // Begin private section of the class declaration.
    private:
      // Declare bool m_is_hex. (inline comment notes: /< indicate that the payload is given by a string of hex values)
      bool m_is_hex;                        ///< indicate that the payload is given by a string of hex values
      // Declare char m_separator. (inline comment notes: /< the separator for file inputs)
      char m_separator;                     ///< the separator for file inputs
      // Declare std::vector<uint8_t> m_payload. (inline comment notes: /< store the payload bytes)
      std::vector<uint8_t> m_payload;       ///< store the payload bytes
      // Declare std::vector<std::string> payload_str. (inline comment notes: /< payload as a string)
      std::vector<std::string> payload_str; ///< payload as a string
      // Declare bool m_file_source. (inline comment notes: /< indicate that the payload are provided by a file through an input stream)
      bool m_file_source;                   ///< indicate that the payload are provided by a file through an input stream
      // Declare bool m_use_length_tag. (inline comment notes: /< whether to use the length tag to separate frames or the separator character)
      bool m_use_length_tag;                ///< whether to use the length tag to separate frames or the separator character
      // Declare std::string m_length_tag_name. (inline comment notes: /< name/key of the length tag)
      std::string m_length_tag_name;        ///< name/key of the length tag
      // Declare int m_input_byte_cnt. (inline comment notes: /< number of bytes from the input already processed)
      int m_input_byte_cnt;                 ///< number of bytes from the input already processed
      // Declare uint64_t m_tag_offset. (inline comment notes: /< offset of the length tag)
      uint64_t m_tag_offset;                ///< offset of the length tag
  
      // Execute statement void msg_handler(pmt::pmt_t message).
      void msg_handler(pmt::pmt_t message);
      // Execute statement void frame_info_handler(pmt::pmt_t frame_info).
      void frame_info_handler(pmt::pmt_t frame_info);


    // Begin public section of the class declaration.
    public:
      // Call whitening_impl with arguments (bool is_hex, bool use_length_tag, char separator, std::string length_tag_name).
      whitening_impl(bool is_hex, bool use_length_tag, char separator, std::string length_tag_name);
      // Call ~whitening_impl with arguments ().
      ~whitening_impl();

      // Where all the action really happens
      // Start declaration for int work with arguments listed on subsequent lines.
      int work(
          // Specify parameter or initializer int noutput_items.
          int noutput_items,
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
#endif /* INCLUDED_LORA_WHITENING_IMPL_H */
