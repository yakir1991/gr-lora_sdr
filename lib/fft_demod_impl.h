
// Compile the following section when INCLUDED_LORA_SDR_FFT_DEMOD_IMPL_H is not defined.
#ifndef INCLUDED_LORA_SDR_FFT_DEMOD_IMPL_H
// Define macro INCLUDED_LORA_SDR_FFT_DEMOD_IMPL_H.
#define INCLUDED_LORA_SDR_FFT_DEMOD_IMPL_H
// #define GRLORA_DEBUG
// #define GRLORA_MEASUREMENTS
//#define GRLORA_SNR_MEASUREMENTS_SAVE
//#define GRLORA_BESSEL_MEASUREMENTS_SAVE
//#define GRLORA_LLR_MEASUREMENTS_SAVE

// Bring in declarations from <gnuradio/lora_sdr/fft_demod.h>.
#include <gnuradio/lora_sdr/fft_demod.h>
// Bring in declarations from <iostream>.
#include <iostream>
// Bring in declarations from <fstream>.
#include <fstream>
// Bring in declarations from <volk/volk.h>.
#include <volk/volk.h>
// Bring in declarations from <gnuradio/io_signature.h>.
#include <gnuradio/io_signature.h>
// Bring in declarations from <gnuradio/lora_sdr/utilities.h>.
#include <gnuradio/lora_sdr/utilities.h>
// Bring in declarations from <gnuradio/lora_sdr/fft_demod.h>.
#include <gnuradio/lora_sdr/fft_demod.h>

// Start namespace gr scope.
namespace gr {
  // Start namespace lora_sdr scope.
  namespace lora_sdr {

    // Declare class fft_demod_impl.
    class fft_demod_impl : public fft_demod
    // Open a new scope block.
    {
    // Begin private section of the class declaration.
    private:
      // Declare uint8_t m_sf. (inline comment notes: /< Spreading factor)
      uint8_t m_sf;           ///< Spreading factor
      // Declare uint8_t m_cr. (inline comment notes: /< Coding rate)
      uint8_t m_cr;           ///< Coding rate
      // Declare bool m_soft_decoding. (inline comment notes: /< Hard/Soft decoding)
      bool m_soft_decoding;   ///< Hard/Soft decoding
      // Declare bool max_log_approx. (inline comment notes: /< use Max-log approximation in LLR formula)
      bool max_log_approx;     ///< use Max-log approximation in LLR formula
      // Declare bool m_new_frame. (inline comment notes: /< To be notify when receive a new frame to estimate SNR)
      bool m_new_frame;       ///< To be notify when receive a new frame to estimate SNR
      // Declare bool m_ldro. (inline comment notes: /< use low datarate optimisation)
      bool m_ldro; ///< use low datarate optimisation
      // Declare unsigned int m_symb_numb. (inline comment notes: /< number of symbols in the frame)
      unsigned int m_symb_numb; ///< number of symbols in the frame
      // Declare unsigned int m_symb_cnt. (inline comment notes: /< number of symbol already output in current frame)
      unsigned int m_symb_cnt; ///< number of symbol already output in current frame

      // Start the body of a do-while loop. (inline comment notes: Signal Power estimation updated at each rx symbol)
      double m_Ps_est = 0;   // Signal Power estimation updated at each rx symbol
      // Start the body of a do-while loop. (inline comment notes: Signal Power estimation updated at each rx symbo)
      double m_Pn_est = 0;   // Signal Power estimation updated at each rx symbo

      // Declare uint32_t m_samples_per_symbol. (inline comment notes: /< Number of samples received per lora symbols)
      uint32_t m_samples_per_symbol;  ///< Number of samples received per lora symbols
      // Declare int CFOint. (inline comment notes: /< integer part of the CFO)
      int CFOint; ///< integer part of the CFO

      // variable used to perform the FFT demodulation
      // Declare std::vector<gr_complex> m_upchirp. (inline comment notes: /< Reference upchirp)
      std::vector<gr_complex> m_upchirp;   ///< Reference upchirp
      // Declare std::vector<gr_complex> m_downchirp. (inline comment notes: /< Reference downchirp)
      std::vector<gr_complex> m_downchirp; ///< Reference downchirp
      // Declare std::vector<gr_complex> m_dechirped. (inline comment notes: /< Dechirped symbol)
      std::vector<gr_complex> m_dechirped; ///< Dechirped symbol
      // Declare std::vector<gr_complex> m_fft. (inline comment notes: /< Result of the FFT)
      std::vector<gr_complex> m_fft;       ///< Result of the FFT

      // Declare std::vector<uint16_t> output. (inline comment notes: /< Stores the value to be outputted once a full bloc has been received)
      std::vector<uint16_t> output;   ///< Stores the value to be outputted once a full bloc has been received
      // Declare std::vector< std::vector<LLR> > LLRs_block. (inline comment notes: /< Stores the LLRs to be outputted once a full bloc has been received)
      std::vector< std::vector<LLR> > LLRs_block; ///< Stores the LLRs to be outputted once a full bloc has been received
      // Declare bool is_header. (inline comment notes: /< Indicate that the first block hasn't been fully received)
      bool is_header;                  ///< Indicate that the first block hasn't been fully received
      // Declare uint8_t block_size. (inline comment notes: /< The number of lora symbol in one block)
      uint8_t block_size;             ///< The number of lora symbol in one block
     
      // Only compile the following section when GRLORA_MEASUREMENTS is defined.
      #ifdef GRLORA_MEASUREMENTS
      // Declare std::ofstream energy_file.
      std::ofstream energy_file;
      // Close the preceding conditional compilation block.
      #endif
      // Only compile the following section when GRLORA_DEBUG is defined.
      #ifdef GRLORA_DEBUG
      // Declare std::ofstream idx_file.
      std::ofstream idx_file;
      // Close the preceding conditional compilation block.
      #endif
      // Only compile the following section when GRLORA_SNR_MEASUREMENTS_SAVE is defined.
      #ifdef GRLORA_SNR_MEASUREMENTS_SAVE
      // Declare std::ofstream SNRestim_file.
      std::ofstream SNRestim_file;
      // Close the preceding conditional compilation block.
      #endif
      // Only compile the following section when GRLORA_BESSEL_MEASUREMENTS_SAVE is defined.
      #ifdef GRLORA_BESSEL_MEASUREMENTS_SAVE
      // Declare std::ofstream bessel_file.
      std::ofstream bessel_file;
      // Close the preceding conditional compilation block.
      #endif

      /**
       *  \brief  Recover the lora symbol value using argmax of the dechirped symbol FFT.
       *
       *  \param  samples
       *          The pointer to the symbol beginning.
       */
      // Execute statement uint16_t get_symbol_val(const gr_complex *samples).
      uint16_t get_symbol_val(const gr_complex *samples);

      /**
       * @brief Set spreading factor and init vector sizes accordingly
       * 
       */
      // Execute statement void set_sf(int sf).
      void set_sf(int sf);

      /**
       *  \brief  Reset the block variables when a new lora packet needs to be decoded.
       */
      // Execute statement void new_frame_handler(int cfo_int).
      void new_frame_handler(int cfo_int);

      /**
       *  \brief  Handles the reception of the coding rate received by the header_decoder block.
       */
      // Execute statement void header_cr_handler(pmt::pmt_t cr).
      void header_cr_handler(pmt::pmt_t cr);

      /**
       *  \brief  Compute the FFT and fill the class attributes
       */
      // Execute statement float* compute_fft_mag(const gr_complex *samples).
      float* compute_fft_mag(const gr_complex *samples);

      /**
       *  \brief  Compute the Log-Likelihood Ratios of the SF nbr of bits
       */
      // Execute statement std::vector<LLR> get_LLRs(const gr_complex *samples).
      std::vector<LLR> get_LLRs(const gr_complex *samples);

     // Begin public section of the class declaration.
     public:
      // Call fft_demod_impl with arguments (bool soft_decoding, bool max_log_approx).
      fft_demod_impl( bool soft_decoding, bool max_log_approx);
      // Call ~fft_demod_impl with arguments ().
      ~fft_demod_impl();

      // Where all the action really happens
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

  // Close the current scope block. (inline comment notes: namespace lora_sdr)
  } // namespace lora_sdr
// Close the current scope block. (inline comment notes: namespace gr)
} // namespace gr

// Close the preceding conditional compilation block.
#endif /* INCLUDED_LORA_SDR_FFT_DEMOD_IMPL_H */
