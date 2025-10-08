// Compile the following section when INCLUDED_LORA_MODULATE_IMPL_H is not defined.
#ifndef INCLUDED_LORA_MODULATE_IMPL_H
// Define macro INCLUDED_LORA_MODULATE_IMPL_H.
#define INCLUDED_LORA_MODULATE_IMPL_H

// Bring in declarations from <gnuradio/lora_sdr/modulate.h>.
#include <gnuradio/lora_sdr/modulate.h>
// Bring in declarations from <gnuradio/io_signature.h>.
#include <gnuradio/io_signature.h>
// Bring in declarations from <iostream>.
#include <iostream>
// Bring in declarations from <fstream>.
#include <fstream>

// Bring in declarations from <gnuradio/lora_sdr/utilities.h>.
#include <gnuradio/lora_sdr/utilities.h>

// #define GR_LORA_PRINT_INFO

// Start namespace gr scope.
namespace gr {
  // Start namespace lora_sdr scope.
  namespace lora_sdr {

    // Declare class modulate_impl.
    class modulate_impl : public modulate
    // Open a new scope block.
    {
     // Begin private section of the class declaration.
     private:
        // Declare uint8_t m_sf. (inline comment notes: /< Transmission spreading factor)
        uint8_t m_sf; ///< Transmission spreading factor
        // Declare uint32_t m_samp_rate. (inline comment notes: /< Transmission sampling rate)
        uint32_t m_samp_rate; ///< Transmission sampling rate
        // Declare uint32_t m_bw. (inline comment notes: /< Transmission bandwidth (Works only for samp_rate=bw))
        uint32_t m_bw; ///< Transmission bandwidth (Works only for samp_rate=bw)
        // Declare uint32_t m_number_of_bins. (inline comment notes: /< number of bin per loar symbol)
        uint32_t m_number_of_bins; ///< number of bin per loar symbol
        // Declare int m_samples_per_symbol. (inline comment notes: /< samples per symbols(Works only for 2^sf))
        int m_samples_per_symbol; ///< samples per symbols(Works only for 2^sf)
        // Declare std::vector<uint16_t> m_sync_words. (inline comment notes: /< sync words (network id))
        std::vector<uint16_t> m_sync_words; ///< sync words (network id) 

        // Declare int m_ninput_items_required. (inline comment notes: /< number of samples required to call this block (forecast))
        int m_ninput_items_required; ///< number of samples required to call this block (forecast)

        // Declare int m_os_factor. (inline comment notes: /< ovesampling factor based on sampling rate and bandwidth)
        int m_os_factor; ///< ovesampling factor based on sampling rate and bandwidth

        // Declare uint32_t m_inter_frame_padding. (inline comment notes: /< length in samples of zero append to each frame)
        uint32_t m_inter_frame_padding; ///< length in samples of zero append to each frame

        // Declare int m_frame_len. (inline comment notes: /< leng of the frame in number of items)
        int m_frame_len;///< leng of the frame in number of items

        // Declare std::vector<gr_complex> m_upchirp. (inline comment notes: /< reference upchirp)
        std::vector<gr_complex> m_upchirp; ///< reference upchirp
        // Declare std::vector<gr_complex> m_downchirp. (inline comment notes: /< reference downchirp)
        std::vector<gr_complex> m_downchirp; ///< reference downchirp

        // Declare uint16_t m_preamb_len. (inline comment notes: /< number of upchirps in the preamble)
        uint16_t m_preamb_len; ///< number of upchirps in the preamble
        // Declare int32_t samp_cnt. (inline comment notes: /< counter of the number of lora samples sent)
        int32_t samp_cnt; ///< counter of the number of lora samples sent
        // Declare int32_t preamb_samp_cnt. (inline comment notes: /< counter of the number of preamble symbols output)
        int32_t preamb_samp_cnt; ///< counter of the number of preamble symbols output
        // Declare uint32_t padd_cnt. (inline comment notes: /< counter of the number of null symbols output after each frame)
        uint32_t padd_cnt; ///< counter of the number of null symbols output after each frame
        // Declare uint64_t frame_cnt. (inline comment notes: /< counter of the number of frame sent)
        uint64_t frame_cnt; ///< counter of the number of frame sent
        // Declare bool frame_end. (inline comment notes: /< indicate that we send a full frame)
        bool frame_end; ///< indicate that we send a full frame

        // Declare tag_t m_config_tag.
        tag_t m_config_tag;
        // Declare tag_t m_framelen_tag.
        tag_t m_framelen_tag;

     // Begin public section of the class declaration.
     public:
      // Call modulate_impl with arguments (uint8_t sf, uint32_t samp_rate, uint32_t bw, std::vector<uint16_t> sync_words, uint32_t frame_zero_padd, uint16_t preamb_len).
      modulate_impl(uint8_t sf, uint32_t samp_rate, uint32_t bw, std::vector<uint16_t> sync_words, uint32_t frame_zero_padd, uint16_t preamb_len);
      // Call ~modulate_impl with arguments ().
      ~modulate_impl();

      // Execute statement void set_sf(uint8_t sf).
      void set_sf(uint8_t sf);

      // Where all the action really happens
      // Execute statement void forecast (int noutput_items, gr_vector_int &ninput_items_required).
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);
      // Execute statement void update_var(int new_sf, int new_bw).
      void update_var(int new_sf, int new_bw);
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
#endif /* INCLUDED_LORA_MODULATE_IMPL_H */
