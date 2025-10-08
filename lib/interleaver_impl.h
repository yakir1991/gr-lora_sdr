// Compile the following section when INCLUDED_LORA_INTERLEAVER_IMPL_H is not defined.
#ifndef INCLUDED_LORA_INTERLEAVER_IMPL_H
// Define macro INCLUDED_LORA_INTERLEAVER_IMPL_H.
#define INCLUDED_LORA_INTERLEAVER_IMPL_H

// Bring in declarations from <gnuradio/lora_sdr/interleaver.h>.
#include <gnuradio/lora_sdr/interleaver.h>
// #define GRLORA_DEBUG

// Start namespace gr scope.
namespace gr {
  // Start namespace lora_sdr scope.
  namespace lora_sdr {

    // Declare class interleaver_impl.
    class interleaver_impl : public interleaver
    // Open a new scope block.
    {
     // Begin private section of the class declaration.
     private:
        // Declare uint8_t m_cr. (inline comment notes: /< Transmission coding rate)
        uint8_t m_cr; ///< Transmission coding rate
        // Declare uint8_t m_sf. (inline comment notes: /< Transmission spreading factor)
        uint8_t m_sf; ///< Transmission spreading factor

        // Declare uint32_t cw_cnt. (inline comment notes: /< count the number of codewords)
        uint32_t cw_cnt; ///< count the number of codewords
        // Declare int m_frame_len. (inline comment notes: /<length of the frame in number of items)
        int m_frame_len; ///<length of the frame in number of items
        // Declare bool m_ldro. (inline comment notes: /< use the low datarate optimisation mode)
        bool m_ldro; ///< use the low datarate optimisation mode
        // Declare int m_bw.
        int m_bw;
        // Declare uint8_t m_ldro_mode. (inline comment notes: /< mode of the low datarate optimisation (0: off, 1: on, 2: auto))
        uint8_t m_ldro_mode; ///< mode of the low datarate optimisation (0: off, 1: on, 2: auto)
        // Declare tag_t m_config_tag.
        tag_t m_config_tag;
        // Declare tag_t m_framelen_tag.
        tag_t m_framelen_tag;
        // Declare bool m_has_config_tag. (inline comment notes: /<indicate that a configuration tag was received)
        bool m_has_config_tag; ///<indicate that a configuration tag was received


     // Begin public section of the class declaration.
     public:
      // Call interleaver_impl with arguments (uint8_t cr, uint8_t sf, uint8_t ldro_mode, int bw).
      interleaver_impl(uint8_t cr, uint8_t sf, uint8_t ldro_mode, int bw);
      // Call ~interleaver_impl with arguments ().
      ~interleaver_impl();

      // Execute statement void set_cr(uint8_t cr).
      void set_cr(uint8_t cr);
      // Execute statement uint8_t get_cr().
      uint8_t get_cr();
      // Execute statement void set_sf(uint8_t sf).
      void set_sf(uint8_t sf);
      // Execute statement void update_var(int new_cr, int new_sf, int new_bw).
      void update_var(int new_cr, int new_sf, int new_bw);
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
#endif /* INCLUDED_LORA_INTERLEAVER_IMPL_H */
