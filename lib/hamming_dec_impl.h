// Compile the following section when INCLUDED_LORA_hamming_dec_IMPL_H is not defined.
#ifndef INCLUDED_LORA_hamming_dec_IMPL_H
// Define macro INCLUDED_LORA_hamming_dec_IMPL_H.
#define INCLUDED_LORA_hamming_dec_IMPL_H

// Bring in declarations from <gnuradio/lora_sdr/hamming_dec.h>.
#include <gnuradio/lora_sdr/hamming_dec.h>

// Start namespace gr scope.
namespace gr {
  // Start namespace lora_sdr scope.
  namespace lora_sdr {

    // Declare class hamming_dec_impl.
    class hamming_dec_impl : public hamming_dec
    // Open a new scope block.
    {
     // Begin private section of the class declaration.
     private:        
        // Declare uint8_t m_cr. (inline comment notes: /< Transmission coding rate)
        uint8_t m_cr;   ///< Transmission coding rate
        // Declare uint8_t cr_app. (inline comment notes: /< Coding rate use for the block)
        uint8_t cr_app; ///< Coding rate use for the block
        // Declare bool is_header. (inline comment notes: /< Indicate that it is the first block)
        bool is_header;  ///< Indicate that it is the first block
        // Declare bool m_soft_decoding. (inline comment notes: /< Hard/Soft decoding)
        bool m_soft_decoding;   ///< Hard/Soft decoding

     // Begin public section of the class declaration.
     public:
      // Call hamming_dec_impl with arguments (bool soft_decoding).
      hamming_dec_impl(bool soft_decoding);
      // Call ~hamming_dec_impl with arguments ().
      ~hamming_dec_impl();

      // Start declaration for int work with arguments listed on subsequent lines.
      int work(
              // Specify parameter or initializer int noutput_items.
              int noutput_items,
              // Specify parameter or initializer gr_vector_const_void_star &input_items.
              gr_vector_const_void_star &input_items,
              // Execute gr_vector_void_star &output_items.
              gr_vector_void_star &output_items
      // Execute statement ).
      );
    // Close the current scope and emit the trailing comment.
    };

  // Close the current scope block. (inline comment notes: namespace lora)
  } // namespace lora
// Close the current scope block. (inline comment notes: namespace gr)
} // namespace gr

// Close the preceding conditional compilation block.
#endif /* INCLUDED_LORA_hamming_dec_IMPL_H */
