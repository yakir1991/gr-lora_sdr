// Compile the following section when INCLUDED_LORA_DEINTERLEAVER_IMPL_H is not defined.
#ifndef INCLUDED_LORA_DEINTERLEAVER_IMPL_H
// Define macro INCLUDED_LORA_DEINTERLEAVER_IMPL_H.
#define INCLUDED_LORA_DEINTERLEAVER_IMPL_H

// #define GRLORA_DEBUG
// Bring in declarations from <gnuradio/lora_sdr/deinterleaver.h>.
#include <gnuradio/lora_sdr/deinterleaver.h>

// Start namespace gr scope.
namespace gr {
  // Start namespace lora_sdr scope.
  namespace lora_sdr {

    // Declare class deinterleaver_impl.
    class deinterleaver_impl : public deinterleaver
    // Open a new scope block.
    {
     // Begin private section of the class declaration.
     private:      
      // Declare uint8_t m_sf. (inline comment notes: /< Transmission Spreading factor)
      uint8_t m_sf;     ///< Transmission Spreading factor
      // Declare uint8_t m_cr. (inline comment notes: /< Transmission Coding rate)
      uint8_t m_cr;     ///< Transmission Coding rate
      // Declare uint8_t sf_app. (inline comment notes: /< Spreading factor to use to deinterleave)
      uint8_t sf_app;   ///< Spreading factor to use to deinterleave
      // Declare uint8_t cw_len. (inline comment notes: /< Length of a codeword)
      uint8_t cw_len;   ///< Length of a codeword
      // Declare bool m_is_header. (inline comment notes: /< Indicate that we need to deinterleave the first block with the default header parameters (cr=4/8, reduced rate))
      bool m_is_header;    ///< Indicate that we need to deinterleave the first block with the default header parameters (cr=4/8, reduced rate)
      // Declare bool m_soft_decoding. (inline comment notes: /< Hard/Soft decoding)
      bool m_soft_decoding;   ///< Hard/Soft decoding
      // Declare bool m_ldro. (inline comment notes: /< use low datarate optimization mode)
      bool m_ldro; ///< use low datarate optimization mode

     // Begin public section of the class declaration.
     public:
      // Call deinterleaver_impl with arguments (bool soft_decoding).
      deinterleaver_impl(bool soft_decoding);
      // Call ~deinterleaver_impl with arguments ().
      ~deinterleaver_impl();

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
#endif /* INCLUDED_LORA_DEINTERLEAVER_IMPL_H */
