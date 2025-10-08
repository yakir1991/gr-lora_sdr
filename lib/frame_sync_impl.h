
// Compile the following section when INCLUDED_LORA_SDR_FRAME_SYNC_IMPL_H is not defined.
#ifndef INCLUDED_LORA_SDR_FRAME_SYNC_IMPL_H
// Define macro INCLUDED_LORA_SDR_FRAME_SYNC_IMPL_H.
#define INCLUDED_LORA_SDR_FRAME_SYNC_IMPL_H
// #define GRLORA_DEBUG
// #define PRINT_INFO

// Bring in declarations from <gnuradio/lora_sdr/frame_sync.h>.
#include <gnuradio/lora_sdr/frame_sync.h>
// Bring in declarations from <iostream>.
#include <iostream>
// Bring in declarations from <fstream>.
#include <fstream>
// Bring in declarations from <volk/volk.h>.
#include <volk/volk.h>
// Bring in declarations from <gnuradio/lora_sdr/utilities.h>.
#include <gnuradio/lora_sdr/utilities.h>
// Bring in declarations from <gnuradio/io_signature.h>.
#include <gnuradio/io_signature.h>
// Execute extern "C".
extern "C"
// Open a new scope block.
{
// Bring in declarations from "kiss_fft.h".
#include "kiss_fft.h"
// Close the current scope block.
}

// Start namespace gr scope.
namespace gr
// Open a new scope block.
{
  // Start namespace lora_sdr scope.
  namespace lora_sdr
  // Open a new scope block.
  {

    // Declare class frame_sync_impl.
    class frame_sync_impl : public frame_sync
    // Open a new scope block.
    {
    // Begin private section of the class declaration.
    private:
      // Declare enumeration DecoderState.
      enum DecoderState
      // Open a new scope block.
      {
        // Specify parameter or initializer DETECT.
        DETECT,
        // Specify parameter or initializer SYNC.
        SYNC,
        // Specify parameter or initializer SFO_COMPENSATION.
        SFO_COMPENSATION,
        // Execute STOP.
        STOP
      // Close the current scope and emit the trailing comment.
      };
      // Declare enumeration SyncState.
      enum SyncState
      // Open a new scope block.
      {
        // Specify parameter or initializer NET_ID1.
        NET_ID1,
        // Specify parameter or initializer NET_ID2.
        NET_ID2,
        // Specify parameter or initializer DOWNCHIRP1.
        DOWNCHIRP1,
        // Specify parameter or initializer DOWNCHIRP2.
        DOWNCHIRP2,
        // Execute QUARTER_DOWN.
        QUARTER_DOWN
      // Close the current scope and emit the trailing comment.
      };
      // Declare uint8_t m_state. (inline comment notes: /< Current state of the synchronization)
      uint8_t m_state;                    ///< Current state of the synchronization
      // Declare uint32_t m_center_freq. (inline comment notes: /< RF center frequency)
      uint32_t m_center_freq;             ///< RF center frequency
      // Declare uint32_t m_bw. (inline comment notes: /< Bandwidth)
      uint32_t m_bw;                      ///< Bandwidth
      // Declare uint32_t m_samp_rate. (inline comment notes: /< Sampling rate)
      uint32_t m_samp_rate;               ///< Sampling rate
      // Declare uint8_t m_sf. (inline comment notes: /< Spreading factor)
      uint8_t m_sf;                       ///< Spreading factor
      // Declare uint8_t m_cr. (inline comment notes: /< Coding rate)
      uint8_t m_cr;                       ///< Coding rate
      // Declare uint32_t m_pay_len. (inline comment notes: /< payload length)
      uint32_t m_pay_len;                 ///< payload length
      // Declare uint8_t m_has_crc. (inline comment notes: /< CRC presence)
      uint8_t m_has_crc;                  ///< CRC presence
      // Declare uint8_t m_invalid_header. (inline comment notes: /< invalid header checksum)
      uint8_t m_invalid_header;           ///< invalid header checksum
      // Declare bool m_impl_head. (inline comment notes: /< use implicit header mode)
      bool m_impl_head;                   ///< use implicit header mode
      // Declare uint8_t m_os_factor. (inline comment notes: /< oversampling factor)
      uint8_t m_os_factor;                ///< oversampling factor
      // Declare std::vector<uint16_t> m_sync_words. (inline comment notes: /< vector containing the two sync words (network identifiers))
      std::vector<uint16_t> m_sync_words; ///< vector containing the two sync words (network identifiers)
      // Declare bool m_ldro. (inline comment notes: /< use of low datarate optimisation mode)
      bool m_ldro;                        ///< use of low datarate optimisation mode

      // Declare uint8_t m_n_up_req. (inline comment notes: /< number of consecutive upchirps required to trigger a detection)
      uint8_t m_n_up_req;            ///< number of consecutive upchirps required to trigger a detection

      // Declare uint32_t m_number_of_bins. (inline comment notes: /< Number of bins in each lora Symbol)
      uint32_t m_number_of_bins;     ///< Number of bins in each lora Symbol
      // Declare uint32_t m_samples_per_symbol. (inline comment notes: /< Number of samples received per lora symbols)
      uint32_t m_samples_per_symbol; ///< Number of samples received per lora symbols
      // Declare uint32_t m_symb_numb. (inline comment notes: /<number of payload lora symbols)
      uint32_t m_symb_numb;          ///<number of payload lora symbols
      // Declare bool m_received_head. (inline comment notes: /< indicate that the header has be decoded and received by this block)
      bool m_received_head;          ///< indicate that the header has be decoded and received by this block
      // Start the body of a do-while loop. (inline comment notes: /< estimate of the noise)
      double m_noise_est;            ///< estimate of the noise

      // Declare std::vector<gr_complex> in_down. (inline comment notes: /< downsampled input)
      std::vector<gr_complex> in_down;     ///< downsampled input
      // Declare std::vector<gr_complex> m_downchirp. (inline comment notes: /< Reference downchirp)
      std::vector<gr_complex> m_downchirp; ///< Reference downchirp
      // Declare std::vector<gr_complex> m_upchirp. (inline comment notes: /< Reference upchirp)
      std::vector<gr_complex> m_upchirp;   ///< Reference upchirp

      // Declare unsigned int frame_cnt. (inline comment notes: /< Number of frame received)
      unsigned int frame_cnt;      ///< Number of frame received
      // Declare int32_t symbol_cnt. (inline comment notes: /< Number of symbols already received)
      int32_t symbol_cnt;  ///< Number of symbols already received
      // Declare int32_t bin_idx. (inline comment notes: /< value of previous lora symbol)
      int32_t bin_idx;     ///< value of previous lora symbol
      // Declare int32_t bin_idx_new. (inline comment notes: /< value of newly demodulated symbol)
      int32_t bin_idx_new; ///< value of newly demodulated symbol

      // Declare uint16_t m_preamb_len. (inline comment notes: /< Number of consecutive upchirps in preamble)
      uint16_t m_preamb_len; ///< Number of consecutive upchirps in preamble
      // Declare uint8_t additional_upchirps. (inline comment notes: /< indicate the number of additional upchirps found in preamble (in addition to the minimum required to trigger a detection))
      uint8_t additional_upchirps; ///< indicate the number of additional upchirps found in preamble (in addition to the minimum required to trigger a detection)

      // Declare kiss_fft_cfg m_kiss_fft_cfg. (inline comment notes: /< FFT configuration for symbols processing)
      kiss_fft_cfg m_kiss_fft_cfg; ///< FFT configuration for symbols processing
      // Declare kiss_fft_cpx *cx_in. (inline comment notes: /<input of the FFT)
      kiss_fft_cpx *cx_in;  ///<input of the FFT
      // Declare kiss_fft_cpx *cx_out. (inline comment notes: /<output of the FFT)
      kiss_fft_cpx *cx_out; ///<output of the FFT

      // Declare int items_to_consume. (inline comment notes: /< Number of items to consume after each iteration of the general_work function)
      int items_to_consume; ///< Number of items to consume after each iteration of the general_work function

      // Declare int one_symbol_off. (inline comment notes: /< indicate that we are offset by one symbol after the preamble)
      int one_symbol_off; ///< indicate that we are offset by one symbol after the preamble 
      // Declare std::vector<gr_complex> additional_symbol_samp. (inline comment notes: /< save the value of the last 1.25 downchirp as it might contain the first payload symbol)
      std::vector<gr_complex> additional_symbol_samp;  ///< save the value of the last 1.25 downchirp as it might contain the first payload symbol
      // Declare std::vector<gr_complex> preamble_raw. (inline comment notes: /<vector containing the preamble upchirps without any synchronization)
      std::vector<gr_complex> preamble_raw;      ///<vector containing the preamble upchirps without any synchronization
      // Declare std::vector<gr_complex> preamble_raw_up. (inline comment notes: /<vector containing the upsampled preamble upchirps without any synchronization)
      std::vector<gr_complex> preamble_raw_up;  ///<vector containing the upsampled preamble upchirps without any synchronization
      // Declare std::vector<gr_complex> downchirp_raw. (inline comment notes: /< vector containing the preamble downchirps without any synchronization)
      std::vector<gr_complex> downchirp_raw;    ///< vector containing the preamble downchirps without any synchronization
      // Declare std::vector<gr_complex> preamble_upchirps. (inline comment notes: /<vector containing the preamble upchirps)
      std::vector<gr_complex> preamble_upchirps; ///<vector containing the preamble upchirps
      // Declare std::vector<gr_complex> net_id_samp. (inline comment notes: /< vector of the oversampled network identifier samples)
      std::vector<gr_complex> net_id_samp;       ///< vector of the oversampled network identifier samples
      // Declare std::vector<int> net_ids. (inline comment notes: /< values of the network identifiers received)
      std::vector<int> net_ids;                  ///< values of the network identifiers received

      // Declare int up_symb_to_use. (inline comment notes: /< number of upchirp symbols to use for CFO and STO frac estimation)
      int up_symb_to_use;              ///< number of upchirp symbols to use for CFO and STO frac estimation
      // Declare int k_hat. (inline comment notes: /< integer part of CFO+STO)
      int k_hat;                       ///< integer part of CFO+STO
      // Declare std::vector<int> preamb_up_vals. (inline comment notes: /< value of the preamble upchirps)
      std::vector<int> preamb_up_vals; ///< value of the preamble upchirps

      // Declare float m_cfo_frac. (inline comment notes: /< fractional part of CFO)
      float m_cfo_frac;                            ///< fractional part of CFO
      // Declare float m_cfo_frac_bernier. (inline comment notes: /< fractional part of CFO using Berniers algo)
      float m_cfo_frac_bernier;                    ///< fractional part of CFO using Berniers algo
      // Declare int m_cfo_int. (inline comment notes: /< integer part of CFO)
      int m_cfo_int;                               ///< integer part of CFO
      // Declare float m_sto_frac. (inline comment notes: /< fractional part of CFO)
      float m_sto_frac;                            ///< fractional part of CFO
      // Declare float sfo_hat. (inline comment notes: /< estimated sampling frequency offset)
      float sfo_hat;                               ///< estimated sampling frequency offset
      // Declare float sfo_cum. (inline comment notes: /< cumulation of the sfo)
      float sfo_cum;                               ///< cumulation of the sfo
      // Declare bool cfo_frac_sto_frac_est. (inline comment notes: /< indicate that the estimation of CFO_frac and STO_frac has been performed)
      bool cfo_frac_sto_frac_est;                  ///< indicate that the estimation of CFO_frac and STO_frac has been performed
      // Declare std::vector<gr_complex> CFO_frac_correc. (inline comment notes: /< cfo frac correction vector)
      std::vector<gr_complex> CFO_frac_correc;     ///< cfo frac correction vector
      // Declare std::vector<gr_complex> CFO_SFO_frac_correc. (inline comment notes: /< correction vector accounting for cfo and sfo)
      std::vector<gr_complex> CFO_SFO_frac_correc; ///< correction vector accounting for cfo and sfo

      // Declare std::vector<gr_complex> symb_corr. (inline comment notes: /< symbol with CFO frac corrected)
      std::vector<gr_complex> symb_corr; ///< symbol with CFO frac corrected
      // Declare int down_val. (inline comment notes: /< value of the preamble downchirps)
      int down_val;                      ///< value of the preamble downchirps
      // Declare int net_id_off. (inline comment notes: /< offset of the network identifier)
      int net_id_off;                    ///< offset of the network identifier

      // Declare bool m_should_log. (inline comment notes: /< indicate that the sync values should be logged)
      bool m_should_log;   ///< indicate that the sync values should be logged
      // Declare float off_by_one_id. (inline comment notes: /< Indicate that the network identifiers where off by one and corrected (float used as saved in a float32 bin file))
      float off_by_one_id; ///< Indicate that the network identifiers where off by one and corrected (float used as saved in a float32 bin file)
// Only compile the following section when GRLORA_DEBUG is defined.
#ifdef GRLORA_DEBUG
      // Declare std::ofstream preamb_file.
      std::ofstream preamb_file;
// Close the preceding conditional compilation block.
#endif
      // std::ofstream start_off_file;
      // std::ofstream netid_file;
      // Execute statement int my_roundf(float number).
      int my_roundf(float number);
      
      /**
          *  \brief  Estimate the value of fractional part of the CFO using RCTSL and correct the received preamble accordingly
          *  \param  samples
          *          The pointer to the preamble beginning.(We might want to avoid the
          *          first symbol since it might be incomplete)
          */
      // Execute statement float estimate_CFO_frac(gr_complex *samples).
      float estimate_CFO_frac(gr_complex *samples);
      /**
          *  \brief  (not used) Estimate the value of fractional part of the CFO using Berniers algorithm and correct the received preamble accordingly
          *  \param  samples
          *          The pointer to the preamble beginning.(We might want to avoid the
          *          first symbol since it might be incomplete)
          */
      // Execute statement float estimate_CFO_frac_Bernier(gr_complex *samples).
      float estimate_CFO_frac_Bernier(gr_complex *samples);
      /**
          *  \brief  Estimate the value of fractional part of the STO from m_consec_up and returns the estimated value
          * 
          **/
      // Execute statement float estimate_STO_frac().
      float estimate_STO_frac();
      /**
          *  \brief  Recover the lora symbol value using argmax of the dechirped symbol FFT. Returns -1 in case of an fft window containing no energy to handle noiseless simulations.
          *
          *  \param  samples
          *          The pointer to the symbol beginning.
          *  \param  ref_chirp
          *          The reference chirp to use to dechirp the lora symbol.
          */
      // Execute statement uint32_t get_symbol_val(const gr_complex *samples, gr_complex *ref_chirp).
      uint32_t get_symbol_val(const gr_complex *samples, gr_complex *ref_chirp);
      

      /**
          *  \brief  Determine the energy of a symbol.
          *
          *  \param  samples
          *          The complex symbol to analyse.
          *          length
          *          The number of LoRa symbols used for the estimation
          */
      // Execute statement float determine_energy(const gr_complex *samples, int length).
      float determine_energy(const gr_complex *samples, int length);

      /**
         *   \brief  Handle the reception of the explicit header information, received from the header_decoder block 
         */
      // Execute statement void frame_info_handler(pmt::pmt_t frame_info).
      void frame_info_handler(pmt::pmt_t frame_info);

      /**
          *  \brief  Handles reception of the noise estimate
          */
      // Execute statement void noise_est_handler(pmt::pmt_t noise_est).
      void noise_est_handler(pmt::pmt_t noise_est);
      /**
          *  \brief  Set new SF received in a tag (used for CRAN)
          */
      // Execute statement void set_sf(int sf).
      void set_sf(int sf);

      // Execute statement float determine_snr(const gr_complex *samples).
      float determine_snr(const gr_complex *samples);

    // Begin public section of the class declaration.
    public:
      // Call frame_sync_impl with arguments (uint32_t center_freq, uint32_t bandwidth, uint8_t sf, bool impl_head, std::vector<uint16_t> sync_word, uint8_t os_factor, uint16_t preamb_len).
      frame_sync_impl(uint32_t center_freq, uint32_t bandwidth, uint8_t sf, bool impl_head, std::vector<uint16_t> sync_word, uint8_t os_factor, uint16_t preamb_len);
      // Call ~frame_sync_impl with arguments ().
      ~frame_sync_impl();

      // Where all the action really happens
      // Execute statement void forecast(int noutput_items, gr_vector_int &ninput_items_required).
      void forecast(int noutput_items, gr_vector_int &ninput_items_required);

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
#endif /* INCLUDED_LORA_SDR_FRAME_SYNC_IMPL_H */
