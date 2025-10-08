// Only compile the following section when HAVE_CONFIG_H is defined.
#ifdef HAVE_CONFIG_H
// Bring in declarations from "config.h".
#include "config.h"
// Close the preceding conditional compilation block.
#endif

// Bring in declarations from <gnuradio/io_signature.h>.
#include <gnuradio/io_signature.h>

// Bring in declarations from <boost/math/special_functions/bessel.hpp>. (inline comment notes: to compute LLR)
#include <boost/math/special_functions/bessel.hpp>  // to compute LLR
// Bring in declarations from <limits>.
#include <limits>

// Bring in declarations from "fft_demod_impl.h".
#include "fft_demod_impl.h"
// Execute extern "C" {.
extern "C" {
// Bring in declarations from "kiss_fft.h".
#include "kiss_fft.h"
// Close the current scope block.
}

// Start namespace gr scope.
namespace gr {
    // Start namespace lora_sdr scope.
    namespace lora_sdr {

        // Specify the shared pointer typedef associated with fft_demod.
        fft_demod::sptr
        // Define the d::make function.
        fft_demod::make( bool soft_decoding, bool max_log_approx) {
            // Return gnuradio::get_initial_sptr(new fft_demod_impl(soft_decoding, max_log_approx)) to the caller.
            return gnuradio::get_initial_sptr(new fft_demod_impl(soft_decoding, max_log_approx));
        // Close the current scope block.
        }

        /*
         * The private constructor
         */
        // Define the l::fft_demod_impl function.
        fft_demod_impl::fft_demod_impl(bool soft_decoding, bool max_log_approx)
            // Initialize base classes or members with gr::block("fft_demod",.
            : gr::block("fft_demod",
                        // Specify parameter or initializer gr::io_signature::make(1, 1, sizeof(gr_complex)).
                        gr::io_signature::make(1, 1, sizeof(gr_complex)),
                        // Specify parameter or initializer gr::io_signature::make(1, 1, soft_decoding ? MAX_SF * sizeof(LLR) : sizeof(uint16_t))).
                        gr::io_signature::make(1, 1, soft_decoding ? MAX_SF * sizeof(LLR) : sizeof(uint16_t))),
                        // Specify parameter or initializer m_soft_decoding(soft_decoding), max_log_approx(max_log_approx).
                        m_soft_decoding(soft_decoding), max_log_approx(max_log_approx), 
                        // Define the e function.
                        m_new_frame(true) {
            // Call set_sf with arguments (MIN_SF). (inline comment notes: accept any new sf)
            set_sf(MIN_SF);//accept any new sf
            // Assign m_symb_cnt to 0.
            m_symb_cnt = 0;
            // m_samples_per_symbol = (uint32_t)(1u << m_sf);
            // Assign m_ldro to false.
            m_ldro = false;

            // Call set_tag_propagation_policy with arguments (TPP_DONT).
            set_tag_propagation_policy(TPP_DONT);
// Only compile the following section when GRLORA_MEASUREMENTS is defined.
#ifdef GRLORA_MEASUREMENTS
            // Assign int num to 0. (inline comment notes: check next file name to use)
            int num = 0;  // check next file name to use
            // Repeat while (1) remains true.
            while (1) {
                // Execute statement std::ifstream infile("../../matlab/measurements/energy" + std::to_string(num) + ".txt").
                std::ifstream infile("../../matlab/measurements/energy" + std::to_string(num) + ".txt");
                // Branch when condition (!infile.good()) evaluates to true.
                if (!infile.good())
                    // Exit the nearest enclosing loop or switch.
                    break;
                // Increment num.
                num++;
            // Close the current scope block.
            }
            // Execute statement energy_file.open("../../matlab/measurements/energy" + std::to_string(num) + ".txt", std::ios::out | std::ios::trunc).
            energy_file.open("../../matlab/measurements/energy" + std::to_string(num) + ".txt", std::ios::out | std::ios::trunc);
// Close the preceding conditional compilation block.
#endif
// Only compile the following section when GRLORA_DEBUG is defined.
#ifdef GRLORA_DEBUG
            // Execute statement idx_file.open("../data/idx.txt", std::ios::out | std::ios::trunc).
            idx_file.open("../data/idx.txt", std::ios::out | std::ios::trunc);
// Close the preceding conditional compilation block.
#endif
// Only compile the following section when GRLORA_SNR_MEASUREMENTS_SAVE is defined.
#ifdef GRLORA_SNR_MEASUREMENTS_SAVE
            // Execute statement SNRestim_file.open("../data/SNR_estimation.txt", std::ios::out | std::ios::trunc). (inline comment notes: std::ios::trunc);)
            SNRestim_file.open("../data/SNR_estimation.txt", std::ios::out | std::ios::trunc); //std::ios::trunc);
            //SNRestim_file << "New exp" << std::endl;
// Close the preceding conditional compilation block.
#endif
// Only compile the following section when GRLORA_BESSEL_MEASUREMENTS_SAVE is defined.
#ifdef GRLORA_BESSEL_MEASUREMENTS_SAVE
            // Execute statement bessel_file.open("../data/BesselArg.txt", std::ios::out | std::ios::trunc).
            bessel_file.open("../data/BesselArg.txt", std::ios::out | std::ios::trunc);
// Close the preceding conditional compilation block.
#endif
        // Close the current scope block.
        }
        /*
         * Our virtual destructor.
         */
        // Define the l::~fft_demod_impl function.
        fft_demod_impl::~fft_demod_impl() {}

        // Define the l::forecast function.
        void fft_demod_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required) {
            // Assign ninput_items_required[0] to m_samples_per_symbol.
            ninput_items_required[0] = m_samples_per_symbol;
        // Close the current scope block.
        }
        // Define the l::set_sf function. (inline comment notes: Set he new sf for the frame)
        void fft_demod_impl::set_sf(int sf){//Set he new sf for the frame
        // std::cout<<"[fft_demod_impl.cc] new sf received "<<sf<<std::endl;
        // Assign m_sf to sf.
        m_sf = sf;
        // Assign m_samples_per_symbol to (uint32_t)(1u << m_sf).
        m_samples_per_symbol = (uint32_t)(1u << m_sf);
        // Resize m_upchirp to m_samples_per_symbol.
        m_upchirp.resize(m_samples_per_symbol);
        // Resize m_downchirp to m_samples_per_symbol.
        m_downchirp.resize(m_samples_per_symbol);

        // FFT demodulation preparations
        // Resize m_fft to m_samples_per_symbol.
        m_fft.resize(m_samples_per_symbol);
        // Resize m_dechirped to m_samples_per_symbol.
        m_dechirped.resize(m_samples_per_symbol);
    // Close the current scope block.
    }


        // Define the l::compute_fft_mag function.
        float *fft_demod_impl::compute_fft_mag(const gr_complex *samples) {
            // Assign float rec_en to 0.
            float rec_en = 0;
            // Assign kiss_fft_cfg cfg to kiss_fft_alloc(m_samples_per_symbol, 0, 0, 0).
            kiss_fft_cfg cfg = kiss_fft_alloc(m_samples_per_symbol, 0, 0, 0);
            // Assign kiss_fft_cpx *cx_in to new kiss_fft_cpx[m_samples_per_symbol].
            kiss_fft_cpx *cx_in = new kiss_fft_cpx[m_samples_per_symbol];
            // Assign kiss_fft_cpx *cx_out to new kiss_fft_cpx[m_samples_per_symbol].
            kiss_fft_cpx *cx_out = new kiss_fft_cpx[m_samples_per_symbol];

            // Multiply with ideal downchirp
            // Call volk_32fc_x2_multiply_32fc with arguments (&m_dechirped[0], samples, &m_downchirp[0], m_samples_per_symbol).
            volk_32fc_x2_multiply_32fc(&m_dechirped[0], samples, &m_downchirp[0], m_samples_per_symbol);
            // Iterate with loop parameters (uint32_t i = 0; i < m_samples_per_symbol; i++).
            for (uint32_t i = 0; i < m_samples_per_symbol; i++) {
                // Assign cx_in[i].r to m_dechirped[i].real().
                cx_in[i].r = m_dechirped[i].real();
                // Assign cx_in[i].i to m_dechirped[i].imag().
                cx_in[i].i = m_dechirped[i].imag();
            // Close the current scope block.
            }
            // do the FFT
            // Call kiss_fft with arguments (cfg, cx_in, cx_out).
            kiss_fft(cfg, cx_in, cx_out);

            // Get magnitude squared
            // Assign float *m_fft_mag_sq to new float[m_samples_per_symbol]. (inline comment notes: /!\ dynamic memory allocation)
            float *m_fft_mag_sq = new float[m_samples_per_symbol];  // /!\ dynamic memory allocation
            // Iterate with loop parameters (uint32_t i = 0u; i < m_samples_per_symbol; i++).
            for (uint32_t i = 0u; i < m_samples_per_symbol; i++) {
                // Assign m_fft_mag_sq[i] to cx_out[i].r * cx_out[i].r + cx_out[i].i * cx_out[i].i.
                m_fft_mag_sq[i] = cx_out[i].r * cx_out[i].r + cx_out[i].i * cx_out[i].i;
                // Assign rec_en + to m_fft_mag_sq[i].
                rec_en += m_fft_mag_sq[i];
            // Close the current scope block.
            }

            // Call free with arguments (cfg).
            free(cfg);
            // Declare delete[] cx_in.
            delete[] cx_in;
            // Declare delete[] cx_out.
            delete[] cx_out;

            // Return m_fft_mag_sq to the caller. (inline comment notes: /!\ delete[] , release memory after !)
            return m_fft_mag_sq;  // /!\ delete[] , release memory after !
        // Close the current scope block.
        }

        // Use in Hard-decoding
        // Define the l::get_symbol_val function.
        uint16_t fft_demod_impl::get_symbol_val(const gr_complex *samples) {
            // Assign float *m_fft_mag_sq to compute_fft_mag(samples).
            float *m_fft_mag_sq = compute_fft_mag(samples);

            // Return argmax
            // Assign uint16_t idx to std::max_element(m_fft_mag_sq, m_fft_mag_sq + m_samples_per_symbol) - m_fft_mag_sq.
            uint16_t idx = std::max_element(m_fft_mag_sq, m_fft_mag_sq + m_samples_per_symbol) - m_fft_mag_sq;
            // std::cout << " hard-dec idx " << idx /*<< " m_fft_mag_sq " << m_fft_mag_sq[0] */<< std::endl;

// Only compile the following section when GRLORA_MEASUREMENTS is defined.
#ifdef GRLORA_MEASUREMENTS
            // Execute statement energy_file << std::fixed << std::setprecision(10) << m_fft_mag_sq[idx] << "," << m_fft_mag_sq[mod(idx - 1, m_samples_per_symbol)] << "," << m_fft_mag_sq[mod(idx + 1, m_samples_per_symbol)] << "," << rec_en << "," << std::endl.
            energy_file << std::fixed << std::setprecision(10) << m_fft_mag_sq[idx] << "," << m_fft_mag_sq[mod(idx - 1, m_samples_per_symbol)] << "," << m_fft_mag_sq[mod(idx + 1, m_samples_per_symbol)] << "," << rec_en << "," << std::endl;
// Close the preceding conditional compilation block.
#endif
            // std::cout<<"SNR est = "<<m_fft_mag_sq[idx]<<","<<rec_en<<","<<10*log10(m_fft_mag_sq[idx]/(rec_en-m_fft_mag_sq[idx]))<<std::endl;

// Only compile the following section when GRLORA_DEBUG is defined.
#ifdef GRLORA_DEBUG
            // Declare idx_file << idx << ", ".
            idx_file << idx << ", ";
// Close the preceding conditional compilation block.
#endif
            //  std::cout<<idx<<", ";
            // Declare delete[] m_fft_mag_sq.
            delete[] m_fft_mag_sq;

            // Return idx to the caller.
            return idx;
        // Close the current scope block.
        }

        // Use in Soft-decoding
        // Define the l::get_LLRs function.
        std::vector<LLR> fft_demod_impl::get_LLRs(const gr_complex *samples) {
            // Assign float *m_fft_mag_sq to compute_fft_mag(samples). (inline comment notes: from dynamic memory alloc)
            float *m_fft_mag_sq = compute_fft_mag(samples); // from dynamic memory alloc

            // Compute LLRs of the SF bits
            // Execute statement std::vector<double> LLs(m_samples_per_symbol). (inline comment notes: 2**sf  Log-Likelihood)
            std::vector<double> LLs(m_samples_per_symbol);   // 2**sf  Log-Likelihood
            // Execute statement std::vector<LLR> LLRs(MAX_SF,0). (inline comment notes: Log-Likelihood Ratios)
            std::vector<LLR> LLRs(MAX_SF,0);        //      Log-Likelihood Ratios

            
            //static double Ps_frame = 0; // Signal Power estimation updated at each rx new frame
            //static double Pn_frame = 0; // Signal Power estimation updated at each rx new frame

            // compute SNR estimate at each received symbol as SNR remains constant during 1 simulation run
            // Estimate signal power
            // Assign int symbol_idx to std::max_element(m_fft_mag_sq, m_fft_mag_sq + m_samples_per_symbol) - m_fft_mag_sq.
            int symbol_idx = std::max_element(m_fft_mag_sq, m_fft_mag_sq + m_samples_per_symbol) - m_fft_mag_sq;

            // Estimate noise power
            // Start the body of a do-while loop.
            double signal_energy = 0;
            // Start the body of a do-while loop.
            double noise_energy = 0;

            // Assign int n_adjacent_bins to 1. (inline comment notes: Put '0' for best accurate SNR estimation but if symbols energy splitted in 2 bins, put '1' for safety)
            int n_adjacent_bins = 1; // Put '0' for best accurate SNR estimation but if symbols energy splitted in 2 bins, put '1' for safety
            // Iterate with loop parameters (int i = 0; i < (int)m_samples_per_symbol; i++).
            for (int i = 0; i < (int)m_samples_per_symbol; i++) {
                // Branch when condition ( mod(std::abs(i - symbol_idx), m_samples_per_symbol-1) < 1 + n_adjacent_bins ) evaluates to true.
                if ( mod(std::abs(i - symbol_idx), m_samples_per_symbol-1) < 1 + n_adjacent_bins ) 
                    // Assign signal_energy + to m_fft_mag_sq[i].
                    signal_energy += m_fft_mag_sq[i];
                // Handle the alternative branch when previous conditions fail.
                else
                    // Assign noise_energy + to m_fft_mag_sq[i].
                    noise_energy += m_fft_mag_sq[i];   
            // Close the current scope block.
            }
            // If you want to use a normalized constant identical to all symbols within a frame, but it leads to same performance
            // Lowpass filter update
            //double p = 0.99; // proportion to keep
            //Ps_est = p*Ps_est + (1-p)*  signal_energy / m_samples_per_symbol;
            //Pn_est = p*Pn_est + (1-p)* noise_energy / (m_samples_per_symbol-1-2*n_adjacent_bins); // remove used bins for better estimation
            // Signal and noise power estimation for each received symbol
            // Assign m_Ps_est to signal_energy / m_samples_per_symbol.
            m_Ps_est = signal_energy / m_samples_per_symbol;
            // Assign m_Pn_est to noise_energy / (m_samples_per_symbol-1-2*n_adjacent_bins).
            m_Pn_est = noise_energy / (m_samples_per_symbol-1-2*n_adjacent_bins);
            
// Only compile the following section when GRLORA_SNR_MEASUREMENTS_SAVE is defined.
#ifdef GRLORA_SNR_MEASUREMENTS_SAVE
            // Execute statement SNRestim_file << std::setprecision(6) << m_Ps_est << "," << m_Pn_est << std::endl.
            SNRestim_file << std::setprecision(6) << m_Ps_est << "," << m_Pn_est << std::endl;
// Close the preceding conditional compilation block.
#endif
            /*static int num_frames = 0;
            if (m_new_frame) { 
                Ps_frame = Ps_est;
                Pn_frame = Pn_est;
                m_new_frame = false; // will be set back to True by new_frame_handler()
                num_frames++;
                //if (num_frames % 100 == 0) std::cout << "-----> SNRdB estim: " << 10*std::log10(Ps_frame/Pn_frame) << std::endl;
            }*/

// Only compile the following section when GRLORA_BESSEL_MEASUREMENTS_SAVE is defined.
#ifdef GRLORA_BESSEL_MEASUREMENTS_SAVE
            // Iterate with loop parameters (uint32_t n = 0; n < m_samples_per_symbol; n++).
            for (uint32_t n = 0; n < m_samples_per_symbol; n++) {
                // Execute statement bessel_file << std::setprecision(8) << std::sqrt(Ps_frame) / Pn_frame * std::sqrt(m_fft_mag_sq[n]) << ","  << Ps_frame << "," << Pn_frame << "," << m_fft_mag_sq[n] << std::endl.
                bessel_file << std::setprecision(8) << std::sqrt(Ps_frame) / Pn_frame * std::sqrt(m_fft_mag_sq[n]) << ","  << Ps_frame << "," << Pn_frame << "," << m_fft_mag_sq[n] << std::endl;
            // Close the current scope block.
            }            
// Close the preceding conditional compilation block.
#endif
            //double SNRdB_estimate = 10*std::log10(Ps_frame/Pn_frame);
            // Start the body of a do-while loop.
            double SNRdB_estimate = 10*std::log10(m_Ps_est/m_Pn_est);
            //std::cout << "SNR " << SNRdB_estimate << std::endl;
            //  Normalize fft_mag to 1 to avoid Bessel overflow
            // Iterate with loop parameters (uint32_t i = 0; i < m_samples_per_symbol; i++). (inline comment notes: upgrade to avoid for loop)
            for (uint32_t i = 0; i < m_samples_per_symbol; i++) {  // upgrade to avoid for loop
                // Assign m_fft_mag_sq[i] * to m_samples_per_symbol. (inline comment notes: Normalized |Y[n]| * sqrt(N) => |Y[n]|² * N (depends on kiss FFT library))
                m_fft_mag_sq[i] *= m_samples_per_symbol; // Normalized |Y[n]| * sqrt(N) => |Y[n]|² * N (depends on kiss FFT library)
                //m_fft_mag_sq[i] /= Ps_frame; // // Normalize to avoid Bessel overflow (does not change the performances)
            // Close the current scope block.
            }

            // Assign bool clipping to false.
            bool clipping = false;
            // Iterate with loop parameters (uint32_t n = 0; n < m_samples_per_symbol; n++).
            for (uint32_t n = 0; n < m_samples_per_symbol; n++) {
                // Start the body of a do-while loop.
                double bessel_arg = std::sqrt(m_Ps_est) / m_Pn_est * std::sqrt(m_fft_mag_sq[n]);
                // Manage overflow of Bessel function
                // Branch when condition (bessel_arg < 713) evaluates to true. (inline comment notes: 713 ~ log(std::numeric_limits<LLR>::max()))
                if (bessel_arg < 713)  // 713 ~ log(std::numeric_limits<LLR>::max())
                    // Assign LLs[n] to boost::math::cyl_bessel_i(0, bessel_arg). (inline comment notes: compute Bessel safely)
                    LLs[n] = boost::math::cyl_bessel_i(0, bessel_arg);  // compute Bessel safely
                // Handle the alternative branch when previous conditions fail.
                else {
                    //std::cerr << RED << "Log-Likelihood clipping :-( SNR: " << SNRdB_estimate << " |Y|: " << std::sqrt(m_fft_mag_sq[n]) << RESET << std::endl;
                    //LLs[n] = std::numeric_limits<LLR>::max();  // clipping
                    // Assign clipping to true.
                    clipping = true;
                // Close the current scope block.
                }
                // Branch when condition (max_log_approx) LLs[n] = std::log(LLs[n]) evaluates to true. (inline comment notes: Log-Likelihood)
                if (max_log_approx) LLs[n] = std::log(LLs[n]);  // Log-Likelihood
                //LLs[n] = m_fft_mag_sq[n]; // same performance with just |Y[n]| or |Y[n]|²
            // Close the current scope block.
            }

            // Branch when condition (clipping) evaluates to true. (inline comment notes: change to max-log formula with only |Y[n]|² to avoid overflows, solve LLR computation incapacity in high SNR)
            if (clipping) // change to max-log formula with only |Y[n]|² to avoid overflows, solve LLR computation incapacity in high SNR
                // Iterate with loop parameters (uint32_t n = 0; n < m_samples_per_symbol; n++).
                for (uint32_t n = 0; n < m_samples_per_symbol; n++) LLs[n] = m_fft_mag_sq[n];

            // Log-Likelihood Ratio estimations
            // Branch when condition (max_log_approx) evaluates to true.
            if (max_log_approx) {
                // Iterate with loop parameters (uint32_t i = 0; i < m_sf; i++). (inline comment notes: sf bits => sf LLRs)
                for (uint32_t i = 0; i < m_sf; i++) { // sf bits => sf LLRs
                    // Start the body of a do-while loop. (inline comment notes: X1 = set of symbols where i-th bit is '1')
                    double max_X1(0), max_X0(0); // X1 = set of symbols where i-th bit is '1'
                    // Iterate with loop parameters (uint32_t n = 0; n < m_samples_per_symbol; n++). (inline comment notes: for all symbols n : 0 --> 2^sf)
                    for (uint32_t n = 0; n < m_samples_per_symbol; n++) {  // for all symbols n : 0 --> 2^sf
                        // LoRa: shift by -1 and use reduce rate if first block (header)
                        // Assign uint32_t s to mod(n - 1, (1 << m_sf)) / ((is_header||m_ldro )? 4 : 1).
                        uint32_t s = mod(n - 1, (1 << m_sf)) / ((is_header||m_ldro )? 4 : 1);
                        // Assign s to (s ^ (s >> 1u)). (inline comment notes: Gray encoding formula               // Gray demap before (in this block))
                        s = (s ^ (s >> 1u));  // Gray encoding formula               // Gray demap before (in this block)
                        // Branch when condition (s & (1u << i)) evaluates to true. (inline comment notes: if i-th bit of symbol n is '1')
                        if (s & (1u << i)) {  // if i-th bit of symbol n is '1'
                            // Branch when condition (LLs[n] > max_X1) evaluates to true.
                            if (LLs[n] > max_X1) max_X1 = LLs[n];
                        // Close the current scope and emit the trailing comment. (inline comment notes: if i-th bit of symbol n is '0')
                        } else {              // if i-th bit of symbol n is '0'
                            // Branch when condition (LLs[n] > max_X0) evaluates to true.
                            if (LLs[n] > max_X0) max_X0 = LLs[n];
                        // Close the current scope block.
                        }
                    // Close the current scope block.
                    }
                    // Assign LLRs[m_sf - 1 - i] to max_X1 - max_X0. (inline comment notes: [MSB ... ... LSB])
                    LLRs[m_sf - 1 - i] = max_X1 - max_X0;  // [MSB ... ... LSB]
                // Close the current scope block.
                }
            // Close the current scope and emit the trailing comment.
            } else {
                // Without max-log approximation of the LLR estimation
                // Iterate with loop parameters (uint32_t i = 0; i < m_sf; i++).
                for (uint32_t i = 0; i < m_sf; i++) {
                    // Start the body of a do-while loop. (inline comment notes: X1 = set of symbols where i-th bit is '1')
                    double sum_X1(0), sum_X0(0); // X1 = set of symbols where i-th bit is '1'
                    // Iterate with loop parameters (uint32_t n = 0; n < m_samples_per_symbol; n++). (inline comment notes: for all symbols n : 0 --> 2^sf)
                    for (uint32_t n = 0; n < m_samples_per_symbol; n++) {  // for all symbols n : 0 --> 2^sf
                        // Assign uint32_t s to mod(n - 1, (1 << m_sf)) / ((is_header||m_ldro)? 4 : 1).
                        uint32_t s = mod(n - 1, (1 << m_sf)) / ((is_header||m_ldro)? 4 : 1);
                        // Assign s to (s ^ (s >> 1u)). (inline comment notes: Gray demap)
                        s = (s ^ (s >> 1u));  // Gray demap
                        // Branch when condition (s & (1u << i)) evaluates to true. (inline comment notes: Likelihood)
                        if (s & (1u << i)) sum_X1 += LLs[n]; // Likelihood
                        // Handle the alternative branch when previous conditions fail.
                        else sum_X0 += LLs[n];
                    // Close the current scope block.
                    }
                    // Assign LLRs[m_sf - 1 - i] to std::log(sum_X1) - std::log(sum_X0). (inline comment notes: [MSB ... ... LSB])
                    LLRs[m_sf - 1 - i] = std::log(sum_X1) - std::log(sum_X0); // [MSB ... ... LSB]
                // Close the current scope block.
                }
            // Close the current scope block.
            }

// Only compile the following section when GRLORA_LLR_MEASUREMENTS_SAVE is defined.
#ifdef GRLORA_LLR_MEASUREMENTS_SAVE
            // Save Log-Likelihood and LLR for debug
            // Declare std::ofstream LL_file, LLR_file.
            std::ofstream LL_file, LLR_file;
            // Execute statement LL_file.open("../data/fft_LL.txt", std::ios::out | std::ios::trunc).
            LL_file.open("../data/fft_LL.txt", std::ios::out | std::ios::trunc);
            // Execute statement LLR_file.open("../data/LLR.txt", std::ios::out | std::ios::trunc).
            LLR_file.open("../data/LLR.txt", std::ios::out | std::ios::trunc);

            // Iterate with loop parameters (uint32_t n = 0; n < m_samples_per_symbol; n++).
            for (uint32_t n = 0; n < m_samples_per_symbol; n++) 
                // Execute statement LL_file << std::fixed << std::setprecision(10) << m_fft_mag_sq[n] << "," << LLs[n] << std::endl.
                LL_file << std::fixed << std::setprecision(10) << m_fft_mag_sq[n] << "," << LLs[n] << std::endl;
            // Execute statement LL_file.close().
            LL_file.close();
            // Iterate with loop parameters (uint32_t i = 0; i < m_sf; i++) LLR_file << std::fixed << std::setprecision(10).
            for (uint32_t i = 0; i < m_sf; i++) LLR_file << std::fixed << std::setprecision(10) << LLRs[i] << std::endl;
            // Execute statement LLR_file.close().
            LLR_file.close();
// Close the preceding conditional compilation block.
#endif

            // Declare delete[] m_fft_mag_sq. (inline comment notes: release memory)
            delete[] m_fft_mag_sq; // release memory
            // Return LLRs to the caller.
            return LLRs;
        // Close the current scope block.
        }

        // Define the l::header_cr_handler function.
        void fft_demod_impl::header_cr_handler(pmt::pmt_t cr) {
            // Assign m_cr to pmt::to_long(cr).
            m_cr = pmt::to_long(cr);
        // Close the current scope and emit the trailing comment.
        };

        // Specify parameter or initializer int fft_demod_impl::general_work(int noutput_items.
        int fft_demod_impl::general_work(int noutput_items,
                                         // Specify parameter or initializer gr_vector_int &ninput_items.
                                         gr_vector_int &ninput_items,
                                         // Specify parameter or initializer gr_vector_const_void_star &input_items.
                                         gr_vector_const_void_star &input_items,
                                         // Execute gr_vector_void_star &output_items) {.
                                         gr_vector_void_star &output_items) {
            // Assign const gr_complex *in to (const gr_complex *)input_items[0].
            const gr_complex *in = (const gr_complex *)input_items[0];
            // Assign uint16_t *out1 to (uint16_t *)output_items[0].
            uint16_t *out1 = (uint16_t *)output_items[0];
            // Assign LLR *out2 to (LLR *)output_items[0].
            LLR *out2 = (LLR *)output_items[0];
            // Assign int to_output to 0.
            int to_output = 0;
            // Declare std::vector<tag_t> tags.
            std::vector<tag_t> tags;
            // Call get_tags_in_window with arguments (tags, 0, 0, m_samples_per_symbol, pmt::string_to_symbol("frame_info")).
            get_tags_in_window(tags, 0, 0, m_samples_per_symbol, pmt::string_to_symbol("frame_info"));
            // Branch when condition (tags.size()) evaluates to true.
            if (tags.size()) 
            // Open a new scope block.
            {
                // Assign pmt::pmt_t err to pmt::string_to_symbol("error").
                pmt::pmt_t err = pmt::string_to_symbol("error");
                // Assign is_header to pmt::to_bool(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("is_header"), err)).
                is_header = pmt::to_bool(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("is_header"), err));
                // Branch when condition (is_header) evaluates to true. (inline comment notes: new frame beginning)
                if (is_header) // new frame beginning
                // Open a new scope block.
                {
                    // Assign int cfo_int to pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("cfo_int"), err)).
                    int cfo_int = pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("cfo_int"), err));
                    // Assign float cfo_frac to pmt::to_float(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("cfo_frac"), err)).
                    float cfo_frac = pmt::to_float(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("cfo_frac"), err));
                    // Assign int sf to pmt::to_double(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("sf"), err)).
                    int sf = pmt::to_double(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("sf"), err));
                    // Branch when condition (sf != m_sf) evaluates to true.
                    if(sf != m_sf)
                        // Call set_sf with arguments (sf).
                        set_sf(sf);
                     //create downchirp taking CFO_int into account
                    // Call build_upchirp with arguments (&m_upchirp[0], mod(cfo_int, m_samples_per_symbol), m_sf).
                    build_upchirp(&m_upchirp[0], mod(cfo_int, m_samples_per_symbol), m_sf);
                    // Call volk_32fc_conjugate_32fc with arguments (&m_downchirp[0], &m_upchirp[0], m_samples_per_symbol).
                    volk_32fc_conjugate_32fc(&m_downchirp[0], &m_upchirp[0], m_samples_per_symbol);
                    // adapt the downchirp to the cfo_frac of the frame
                    // Iterate with loop parameters (uint32_t n = 0; n < m_samples_per_symbol; n++).
                    for (uint32_t n = 0; n < m_samples_per_symbol; n++)
                    // Open a new scope block.
                    {
                        // Assign m_downchirp[n] to m_downchirp[n] * gr_expj(-2 * M_PI * cfo_frac / m_samples_per_symbol * n).
                        m_downchirp[n] = m_downchirp[n] * gr_expj(-2 * M_PI * cfo_frac / m_samples_per_symbol * n);
                    // Close the current scope block.
                    }
                    // Clear all contents from output.
                    output.clear();
                // Close the current scope block.
                } 
                // Handle the alternative branch when previous conditions fail.
                else
                // Open a new scope block.
                {
                    // Assign m_cr to pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("cr"), err)).
                    m_cr = pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("cr"), err));
                    // Assign m_ldro to pmt::to_bool(pmt::dict_ref(tags[0].value,pmt::string_to_symbol("ldro"),err)).
                    m_ldro = pmt::to_bool(pmt::dict_ref(tags[0].value,pmt::string_to_symbol("ldro"),err));
                    // Assign m_symb_numb to pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("symb_numb"), err)).
                    m_symb_numb = pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("symb_numb"), err));                
                // Close the current scope block.
                }
            // Close the current scope block.
            }
            // Branch when condition ((uint32_t)ninput_items[0]>=m_samples_per_symbol) evaluates to true. (inline comment notes: check if we have enough samples at the input)
            if((uint32_t)ninput_items[0]>=m_samples_per_symbol)//check if we have enough samples at the input
            // Open a new scope block.
            {
                // Branch when condition (tags.size()) evaluates to true.
                if (tags.size()){
                        // Assign tags[0].offset to nitems_written(0).
                        tags[0].offset = nitems_written(0);
                        // Call add_item_tag with arguments (0, tags[0]). (inline comment notes: 8 LoRa symbols in the header)
                        add_item_tag(0, tags[0]);  // 8 LoRa symbols in the header
                // Close the current scope block.
                }

                // Assign block_size to 4 + (is_header ? 4 : m_cr).
                block_size = 4 + (is_header ? 4 : m_cr);

                // Branch when condition (m_soft_decoding) evaluates to true.
                if (m_soft_decoding) {
                    // Append get_LLRs(in) to LLRs_block. (inline comment notes: Store 'sf' LLRs)
                    LLRs_block.push_back(get_LLRs(in));  // Store 'sf' LLRs
                // Close the current scope and emit the trailing comment. (inline comment notes: Hard decoding)
                } else {                                 // Hard decoding
                    // shift by -1 and use reduce rate if first block (header)
                    // Append mod(get_symbol_val(in) - 1, (1 << m_sf)) / ((is_header||m_ldro) ? 4 : 1) to output.
                    output.push_back(mod(get_symbol_val(in) - 1, (1 << m_sf)) / ((is_header||m_ldro) ? 4 : 1));
                // Close the current scope block.
                }

                // Branch when condition (output.size() == block_size || LLRs_block.size() == block_size) evaluates to true.
                if (output.size() == block_size || LLRs_block.size() == block_size) {
                    // Branch when condition (m_soft_decoding) evaluates to true.
                    if (m_soft_decoding) {
                        // Iterate with loop parameters (int i = 0; i < block_size; i++).
                        for (int i = 0; i < block_size; i++)
                            // Call memcpy with arguments (out2 + i * MAX_SF, LLRs_block[i].data(), m_sf * sizeof(LLR)).
                            memcpy(out2 + i * MAX_SF, LLRs_block[i].data(), m_sf * sizeof(LLR));
                        // Clear all contents from LLRs_block.
                        LLRs_block.clear();
                    // Close the current scope and emit the trailing comment. (inline comment notes: Hard decoding)
                    } else {  // Hard decoding
                        // Call memcpy with arguments (out1, output.data(), block_size * sizeof(uint16_t)).
                        memcpy(out1, output.data(), block_size * sizeof(uint16_t));
                        // Clear all contents from output.
                        output.clear();
                    // Close the current scope block.
                    }
                    // Assign to_output to block_size.
                    to_output = block_size;
                // Close the current scope block.
                } 
                // Handle the alternative branch when previous conditions fail.
                else
                // Open a new scope block.
                {
                    // Assign to_output to 0.
                    to_output = 0;
                // Close the current scope block.
                }
                // Call consume_each with arguments (m_samples_per_symbol).
                consume_each(m_samples_per_symbol);
                // Assign m_symb_cnt + to 1.
                m_symb_cnt += 1;
                // Branch when condition (m_symb_cnt == m_symb_numb) evaluates to true.
                if(m_symb_cnt == m_symb_numb){
                // std::cout<<"fft_demod_impl.cc end of frame\n";
                // set_sf(0);
                // Assign m_symb_cnt to 0.
                m_symb_cnt = 0;
                // Close the current scope block.
                }
            // Close the current scope block.
            }
            // Handle the alternative branch when previous conditions fail.
            else{
                // Assign to_output to 0.
                to_output = 0;
            // Close the current scope block.
            }
            // Branch when condition (noutput_items < to_output) evaluates to true.
            if (noutput_items < to_output)
            // Open a new scope block.
            {
                // Call print with arguments (RED<<"fft_demod not enough space in output buffer!!"<<RESET).
                print(RED<<"fft_demod not enough space in output buffer!!"<<RESET);
            // Close the current scope block.
            }
            
            // Return to_output to the caller.
            return to_output;
        // Close the current scope block.
        }

    // Close the current scope and emit the trailing comment.
    } /* namespace lora_sdr */
// Close the current scope and emit the trailing comment.
} /* namespace gr */
