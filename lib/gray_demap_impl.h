// Compile the following section when INCLUDED_LORA_GRAY_DEMAP_IMPL_H is not defined.
#ifndef INCLUDED_LORA_GRAY_DEMAP_IMPL_H
// Define macro INCLUDED_LORA_GRAY_DEMAP_IMPL_H.
#define INCLUDED_LORA_GRAY_DEMAP_IMPL_H


// Bring in declarations from <gnuradio/lora_sdr/gray_demap.h>.
#include <gnuradio/lora_sdr/gray_demap.h>

// Start namespace gr scope.
namespace gr {
  // Start namespace lora_sdr scope.
  namespace lora_sdr {

    // Declare class gray_demap_impl.
    class gray_demap_impl : public gray_demap
    // Open a new scope block.
    {
     // Begin private section of the class declaration.
     private:
      // Declare uint8_t m_sf.
      uint8_t m_sf;

     // Begin public section of the class declaration.
     public:
      // Call gray_demap_impl with arguments (uint8_t sf).
      gray_demap_impl(uint8_t sf);
      // Call ~gray_demap_impl with arguments ().
      ~gray_demap_impl();
      // Execute statement void set_sf(uint8_t sf).
      void set_sf(uint8_t sf);
      // Execute statement void frame_info_handler(pmt::pmt_t frame_info).
      void frame_info_handler(pmt::pmt_t frame_info);

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
#endif /* INCLUDED_LORA_GRAY_DEMAP_IMPL_H */
