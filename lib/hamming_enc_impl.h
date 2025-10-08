// Compile the following section when INCLUDED_LORA_HAMMING_ENC_IMPL_H is not defined.
#ifndef INCLUDED_LORA_HAMMING_ENC_IMPL_H
// Define macro INCLUDED_LORA_HAMMING_ENC_IMPL_H.
#define INCLUDED_LORA_HAMMING_ENC_IMPL_H

// Bring in declarations from <gnuradio/lora_sdr/hamming_enc.h>.
#include <gnuradio/lora_sdr/hamming_enc.h>

// Start namespace gr scope.
namespace gr {
  // Start namespace lora_sdr scope.
  namespace lora_sdr {

    // Declare class hamming_enc_impl.
    class hamming_enc_impl : public hamming_enc
    // Open a new scope block.
    {
     // Begin private section of the class declaration.
     private:
        // Declare uint8_t m_cr. (inline comment notes: /< Transmission coding rate)
        uint8_t m_cr; ///< Transmission coding rate
        // Declare uint8_t m_sf. (inline comment notes: /< Transmission spreading factor)
        uint8_t m_sf; ///< Transmission spreading factor
        // Declare int m_cnt. (inline comment notes: /< count the number of processed items in the current frame)
        int m_cnt; ///< count the number of processed items in the current frame

     // Begin public section of the class declaration.
     public:
      // Call hamming_enc_impl with arguments (uint8_t cr, uint8_t sf).
      hamming_enc_impl(uint8_t cr, uint8_t sf);
      // Call ~hamming_enc_impl with arguments ().
      ~hamming_enc_impl();


      // Execute statement void set_cr(uint8_t cr).
      void set_cr(uint8_t cr);
      // Execute statement uint8_t get_cr().
      uint8_t get_cr();
      // Execute statement void set_sf(uint8_t sf).
      void set_sf(uint8_t sf);


      // Where all the action really happens
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
#endif /* INCLUDED_LORA_HAMMING_ENC_IMPL_H */
