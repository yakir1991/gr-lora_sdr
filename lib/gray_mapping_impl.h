// Compile the following section when INCLUDED_LORA_GRAY_MAPPING_IMPL_H is not defined.
#ifndef INCLUDED_LORA_GRAY_MAPPING_IMPL_H
// Define macro INCLUDED_LORA_GRAY_MAPPING_IMPL_H.
#define INCLUDED_LORA_GRAY_MAPPING_IMPL_H
// #define GRLORA_DEBUG
// Bring in declarations from <gnuradio/lora_sdr/gray_mapping.h>.
#include <gnuradio/lora_sdr/gray_mapping.h>


// Start namespace gr scope.
namespace gr {
  // Start namespace lora_sdr scope.
  namespace lora_sdr {

    // Declare class gray_mapping_impl.
    class gray_mapping_impl : public gray_mapping
    // Open a new scope block.
    {
     // Begin private section of the class declaration.
     private:
      // Declare uint8_t m_sf. (inline comment notes: /< Spreading factor)
      uint8_t m_sf;           ///< Spreading factor
      // Declare bool m_soft_decoding. (inline comment notes: /< Hard/Soft decoding)
      bool m_soft_decoding;   ///< Hard/Soft decoding

     // Begin public section of the class declaration.
     public:
      // Call gray_mapping_impl with arguments (bool soft_decoding).
      gray_mapping_impl(bool soft_decoding);
      // Call ~gray_mapping_impl with arguments ().
      ~gray_mapping_impl();

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
#endif /* INCLUDED_LORA_GRAY_MAPPING_IMPL_H */
