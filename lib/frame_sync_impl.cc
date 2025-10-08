// Only compile the following section when HAVE_CONFIG_H is defined.
#ifdef HAVE_CONFIG_H
// Bring in declarations from "config.h".
#include "config.h"
// Close the preceding conditional compilation block.
#endif

// Bring in declarations from <algorithm>.
#include <algorithm>
// Bring in declarations from <vector>.
#include <vector>
// Bring in declarations from <gnuradio/io_signature.h>.
#include <gnuradio/io_signature.h>
// Bring in declarations from <volk/volk_alloc.hh>.
#include <volk/volk_alloc.hh>
// Bring in declarations from "frame_sync_impl.h".
#include "frame_sync_impl.h"

// Start namespace gr scope.
namespace gr
// Open a new scope block.
{
    // Start namespace lora_sdr scope.
    namespace lora_sdr
    // Open a new scope block.
    {

        // Specify the shared pointer typedef associated with frame_sync.
        frame_sync::sptr
        // Define the c::make function.
        frame_sync::make(uint32_t center_freq, uint32_t bandwidth, uint8_t sf, bool impl_head, std::vector<uint16_t> sync_word, uint8_t os_factor, uint16_t preamble_len = 8)
        // Open a new scope block.
        {
            // Return gnuradio::get_initial_sptr(new frame_sync_impl(center_freq, bandwidth, sf, impl_head, sync_word, os_factor, preamble_len)) to the caller.
            return gnuradio::get_initial_sptr(new frame_sync_impl(center_freq, bandwidth, sf, impl_head, sync_word, os_factor, preamble_len));
        // Close the current scope block.
        }

        /*
         * The private constructor
         */
        // Define the l::frame_sync_impl function.
        frame_sync_impl::frame_sync_impl(uint32_t center_freq, uint32_t bandwidth, uint8_t sf, bool impl_head, std::vector<uint16_t> sync_word, uint8_t os_factor, uint16_t preamble_len)
            // Initialize base classes or members with gr::block("frame_sync",.
            : gr::block("frame_sync",
                        // Specify parameter or initializer gr::io_signature::make(1, 1, sizeof(gr_complex)).
                        gr::io_signature::make(1, 1, sizeof(gr_complex)),
                        // Define the e::make2 function.
                        gr::io_signature::make2(1, 2, sizeof(gr_complex), sizeof(float)))
        // Open a new scope block.
        {
            // Assign m_state to DETECT.
            m_state = DETECT;
            // Assign m_center_freq to center_freq.
            m_center_freq = center_freq;
            // Assign m_bw to bandwidth.
            m_bw = bandwidth;
            // Assign m_sf to sf.
            m_sf = sf;

            // Assign m_sync_words to sync_word.
            m_sync_words = sync_word;
            // Assign m_os_factor to os_factor.
            m_os_factor = os_factor;
            // Branch when condition (preamble_len < 5) evaluates to true.
            if (preamble_len < 5)
            // Open a new scope block.
            {
                // Declare std::cerr << RED << " Preamble length should be greater than 5!" << RESET << std::endl.
                std::cerr << RED << " Preamble length should be greater than 5!" << RESET << std::endl;
            // Close the current scope block.
            }

            // Assign m_preamb_len to preamble_len.
            m_preamb_len = preamble_len;
            // Resize net_ids to 2, 0.
            net_ids.resize(2, 0);

            // Assign m_n_up_req to preamble_len - 3.
            m_n_up_req = preamble_len - 3;
            // Assign up_symb_to_use to m_n_up_req - 1.
            up_symb_to_use = m_n_up_req - 1;

            // Assign m_sto_frac to 0.0.
            m_sto_frac = 0.0;

            // Assign m_impl_head to impl_head.
            m_impl_head = impl_head;

            // Convert given sync word into the two modulated values in preamble
            // Branch when condition (m_sync_words.size() == 1) evaluates to true.
            if (m_sync_words.size() == 1)
            // Open a new scope block.
            {
                // Assign uint16_t tmp to m_sync_words[0].
                uint16_t tmp = m_sync_words[0];
                // Resize m_sync_words to 2, 0.
                m_sync_words.resize(2, 0);
                // Assign m_sync_words[0] to ((tmp & 0xF0) >> 4) << 3.
                m_sync_words[0] = ((tmp & 0xF0) >> 4) << 3;
                // Assign m_sync_words[1] to (tmp & 0x0F) << 3.
                m_sync_words[1] = (tmp & 0x0F) << 3;
            // Close the current scope block.
            }

            // Assign m_number_of_bins to (uint32_t)(1u << m_sf).
            m_number_of_bins = (uint32_t)(1u << m_sf);
            // Assign m_samples_per_symbol to m_number_of_bins * m_os_factor.
            m_samples_per_symbol = m_number_of_bins * m_os_factor;
            // Resize additional_symbol_samp to 2 * m_samples_per_symbol.
            additional_symbol_samp.resize(2 * m_samples_per_symbol);
            // Resize m_upchirp to m_number_of_bins.
            m_upchirp.resize(m_number_of_bins);
            // Resize m_downchirp to m_number_of_bins.
            m_downchirp.resize(m_number_of_bins);
            // Resize preamble_upchirps to m_preamb_len * m_number_of_bins.
            preamble_upchirps.resize(m_preamb_len * m_number_of_bins);
            // Resize preamble_raw_up to (m_preamb_len + 3) * m_samples_per_symbol.
            preamble_raw_up.resize((m_preamb_len + 3) * m_samples_per_symbol);
            // Resize CFO_frac_correc to m_number_of_bins.
            CFO_frac_correc.resize(m_number_of_bins);
            // Resize CFO_SFO_frac_correc to m_number_of_bins.
            CFO_SFO_frac_correc.resize(m_number_of_bins);
            // Resize symb_corr to m_number_of_bins.
            symb_corr.resize(m_number_of_bins);
            // Resize in_down to m_number_of_bins.
            in_down.resize(m_number_of_bins);
            // Resize preamble_raw to m_preamb_len * m_number_of_bins.
            preamble_raw.resize(m_preamb_len * m_number_of_bins);
            // Resize net_id_samp to m_samples_per_symbol * 2.5. (inline comment notes: we should be able to move up to one quarter of symbol in each direction)
            net_id_samp.resize(m_samples_per_symbol * 2.5); // we should be able to move up to one quarter of symbol in each direction

            // Call build_ref_chirps with arguments (&m_upchirp[0], &m_downchirp[0], m_sf).
            build_ref_chirps(&m_upchirp[0], &m_downchirp[0], m_sf);

            // Assign bin_idx to 0.
            bin_idx = 0;
            // Assign symbol_cnt to 1.
            symbol_cnt = 1;
            // Assign k_hat to 0.
            k_hat = 0;
            // Resize preamb_up_vals to m_n_up_req, 0.
            preamb_up_vals.resize(m_n_up_req, 0);
            // Assign frame_cnt to 0.
            frame_cnt = 0;
            // Assign m_symb_numb to 0.
            m_symb_numb = 0;

            // Assign m_kiss_fft_cfg to kiss_fft_alloc(m_number_of_bins, 0, 0, 0).
            m_kiss_fft_cfg = kiss_fft_alloc(m_number_of_bins, 0, 0, 0);
            // Assign cx_in to new kiss_fft_cpx[m_number_of_bins].
            cx_in = new kiss_fft_cpx[m_number_of_bins];
            // Assign cx_out to new kiss_fft_cpx[m_number_of_bins].
            cx_out = new kiss_fft_cpx[m_number_of_bins];
            // register message ports
            // Call message_port_register_in with arguments (pmt::mp("frame_info")).
            message_port_register_in(pmt::mp("frame_info"));
            // Define the r function.
            set_msg_handler(pmt::mp("frame_info"), [this](pmt::pmt_t msg)
                            // Execute statement { this->frame_info_handler(msg); }).
                            { this->frame_info_handler(msg); });

            // Call message_port_register_in with arguments (pmt::mp("noise_est")).
            message_port_register_in(pmt::mp("noise_est"));
            // Define the r function.
            set_msg_handler(pmt::mp("noise_est"), [this](pmt::pmt_t msg)
                            // Execute statement { this->noise_est_handler(msg); }).
                            { this->noise_est_handler(msg); });

// Only compile the following section when GRLORA_DEBUG is defined.
#ifdef GRLORA_DEBUG
            // Execute statement preamb_file.open("../../matlab/SFO/preamb.txt", std::ios::out | std::ios::trunc).
            preamb_file.open("../../matlab/SFO/preamb.txt", std::ios::out | std::ios::trunc);
// Close the preceding conditional compilation block.
#endif
            // start_off_file.open("../../matlab/Raspi_HAT/start_off_file.txt", std::ios::out | std::ios::trunc);
            // detect_file.open("../../matlab/SFO/detect.txt", std::ios::out | std::ios::trunc);
            // netid_file.open("../../matlab/SFO/net_id.txt", std::ios::out | std::ios::trunc);
            // netid_corr_file.open("../../matlab/SFO/net_id_corr.txt", std::ios::out | std::ios::trunc);
        // Close the current scope block.
        }

        /*
         * Our virtual destructor.
         */
        // Define the l::~frame_sync_impl function.
        frame_sync_impl::~frame_sync_impl()
        // Open a new scope block.
        {
            // Declare delete[] cx_out.
            delete[] cx_out;
            // Declare delete[] cx_in.
            delete[] cx_in;
            // Call kiss_fft_free with arguments (m_kiss_fft_cfg).
            kiss_fft_free(m_kiss_fft_cfg);
        // Close the current scope block.
        }
        // Define the l::my_roundf function.
        int frame_sync_impl::my_roundf(float number)
        // Open a new scope block.
        {
            // Assign int ret_val to (int)(number > 0 ? int(number + 0.5) : std::ceil(number - 0.5)).
            int ret_val = (int)(number > 0 ? int(number + 0.5) : std::ceil(number - 0.5));
            // Return ret_val to the caller.
            return ret_val;
        // Close the current scope block.
        }
        // Define the l::forecast function.
        void frame_sync_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required)
        // Open a new scope block.
        {
            // Assign ninput_items_required[0] to (m_os_factor * (m_number_of_bins + 2)).
            ninput_items_required[0] = (m_os_factor * (m_number_of_bins + 2));
        // Close the current scope block.
        }

        // Define the l::estimate_CFO_frac function.
        float frame_sync_impl::estimate_CFO_frac(gr_complex *samples)
        // Open a new scope block.
        {
            // Declare int k0.
            int k0;
            // Declare float cfo_frac.
            float cfo_frac;
            // Start the body of a do-while loop.
            double Y_1, Y0, Y1, u, v, ka, wa, k_residual;
            // Execute statement std::vector<gr_complex> CFO_frac_correc_aug(up_symb_to_use * m_number_of_bins). (inline comment notes: /< CFO frac correction vector)
            std::vector<gr_complex> CFO_frac_correc_aug(up_symb_to_use * m_number_of_bins); ///< CFO frac correction vector
            // Execute statement std::vector<gr_complex> dechirped(up_symb_to_use * m_number_of_bins).
            std::vector<gr_complex> dechirped(up_symb_to_use * m_number_of_bins);
            // Assign kiss_fft_cpx *cx_in_cfo to new kiss_fft_cpx[2 * up_symb_to_use * m_number_of_bins].
            kiss_fft_cpx *cx_in_cfo = new kiss_fft_cpx[2 * up_symb_to_use * m_number_of_bins];
            // Assign kiss_fft_cpx *cx_out_cfo to new kiss_fft_cpx[2 * up_symb_to_use * m_number_of_bins].
            kiss_fft_cpx *cx_out_cfo = new kiss_fft_cpx[2 * up_symb_to_use * m_number_of_bins];

            // Execute statement std::vector<float> fft_mag_sq(2 * up_symb_to_use * m_number_of_bins).
            std::vector<float> fft_mag_sq(2 * up_symb_to_use * m_number_of_bins);
            // Assign kiss_fft_cfg cfg_cfo to kiss_fft_alloc(2 * up_symb_to_use * m_number_of_bins, 0, 0, 0).
            kiss_fft_cfg cfg_cfo = kiss_fft_alloc(2 * up_symb_to_use * m_number_of_bins, 0, 0, 0);
            // create longer downchirp
            // Execute statement std::vector<gr_complex> downchirp_aug(up_symb_to_use * m_number_of_bins).
            std::vector<gr_complex> downchirp_aug(up_symb_to_use * m_number_of_bins);
            // Iterate with loop parameters (int i = 0; i < up_symb_to_use; i++).
            for (int i = 0; i < up_symb_to_use; i++)
            // Open a new scope block.
            {
                // Call memcpy with arguments (&downchirp_aug[i * m_number_of_bins], &m_downchirp[0], m_number_of_bins * sizeof(gr_complex)).
                memcpy(&downchirp_aug[i * m_number_of_bins], &m_downchirp[0], m_number_of_bins * sizeof(gr_complex));
            // Close the current scope block.
            }

            // Dechirping
            // Call volk_32fc_x2_multiply_32fc with arguments (&dechirped[0], samples, &downchirp_aug[0], up_symb_to_use * m_number_of_bins).
            volk_32fc_x2_multiply_32fc(&dechirped[0], samples, &downchirp_aug[0], up_symb_to_use * m_number_of_bins);
            // prepare FFT
            // Iterate with loop parameters (uint32_t i = 0; i < 2 * up_symb_to_use * m_number_of_bins; i++).
            for (uint32_t i = 0; i < 2 * up_symb_to_use * m_number_of_bins; i++)
            // Open a new scope block.
            {
                // Branch when condition (i < up_symb_to_use * m_number_of_bins) evaluates to true.
                if (i < up_symb_to_use * m_number_of_bins)
                // Open a new scope block.
                {
                    // Assign cx_in_cfo[i].r to dechirped[i].real().
                    cx_in_cfo[i].r = dechirped[i].real();
                    // Assign cx_in_cfo[i].i to dechirped[i].imag().
                    cx_in_cfo[i].i = dechirped[i].imag();
                // Close the current scope block.
                }
                // Handle the alternative branch when previous conditions fail.
                else
                // Open a new scope block. (inline comment notes: add padding)
                { // add padding
                    // Assign cx_in_cfo[i].r to 0.
                    cx_in_cfo[i].r = 0;
                    // Assign cx_in_cfo[i].i to 0.
                    cx_in_cfo[i].i = 0;
                // Close the current scope block.
                }
            // Close the current scope block.
            }
            // do the FFT
            // Call kiss_fft with arguments (cfg_cfo, cx_in_cfo, cx_out_cfo).
            kiss_fft(cfg_cfo, cx_in_cfo, cx_out_cfo);
            // Get magnitude
            // Iterate with loop parameters (uint32_t i = 0u; i < 2 * up_symb_to_use * m_number_of_bins; i++).
            for (uint32_t i = 0u; i < 2 * up_symb_to_use * m_number_of_bins; i++)
            // Open a new scope block.
            {
                // Assign fft_mag_sq[i] to cx_out_cfo[i].r * cx_out_cfo[i].r + cx_out_cfo[i].i * cx_out_cfo[i].i.
                fft_mag_sq[i] = cx_out_cfo[i].r * cx_out_cfo[i].r + cx_out_cfo[i].i * cx_out_cfo[i].i;
            // Close the current scope block.
            }
            // Call free with arguments (cfg_cfo).
            free(cfg_cfo);
            // Declare delete[] cx_in_cfo.
            delete[] cx_in_cfo;
            // Declare delete[] cx_out_cfo.
            delete[] cx_out_cfo;
            // get argmax here
            // Assign k0 to std::distance(std::begin(fft_mag_sq), std::max_element(std::begin(fft_mag_sq), std::end(fft_mag_sq))).
            k0 = std::distance(std::begin(fft_mag_sq), std::max_element(std::begin(fft_mag_sq), std::end(fft_mag_sq)));

            // get three spectral lines
            // Assign Y_1 to fft_mag_sq[mod(k0 - 1, 2 * up_symb_to_use * m_number_of_bins)].
            Y_1 = fft_mag_sq[mod(k0 - 1, 2 * up_symb_to_use * m_number_of_bins)];
            // Assign Y0 to fft_mag_sq[k0].
            Y0 = fft_mag_sq[k0];
            // Assign Y1 to fft_mag_sq[mod(k0 + 1, 2 * up_symb_to_use * m_number_of_bins)].
            Y1 = fft_mag_sq[mod(k0 + 1, 2 * up_symb_to_use * m_number_of_bins)];
            // set constant coeff
            // Assign u to 64 * m_number_of_bins / 406.5506497. (inline comment notes: from Cui yang (15))
            u = 64 * m_number_of_bins / 406.5506497; // from Cui yang (15)
            // Assign v to u * 2.4674.
            v = u * 2.4674;
            // RCTSL
            // Assign wa to (Y1 - Y_1) / (u * (Y1 + Y_1) + v * Y0).
            wa = (Y1 - Y_1) / (u * (Y1 + Y_1) + v * Y0);
            // Assign ka to wa * m_number_of_bins / M_PI.
            ka = wa * m_number_of_bins / M_PI;
            // Assign k_residual to fmod((k0 + ka) / 2 / up_symb_to_use, 1).
            k_residual = fmod((k0 + ka) / 2 / up_symb_to_use, 1);
            // Assign cfo_frac to k_residual - (k_residual > 0.5 ? 1 : 0).
            cfo_frac = k_residual - (k_residual > 0.5 ? 1 : 0);
            // Correct CFO frac in preamble
            // Iterate with loop parameters (uint32_t n = 0; n < up_symb_to_use * m_number_of_bins; n++).
            for (uint32_t n = 0; n < up_symb_to_use * m_number_of_bins; n++)
            // Open a new scope block.
            {
                // Assign CFO_frac_correc_aug[n] to gr_expj(-2 * M_PI * (cfo_frac) / m_number_of_bins * n).
                CFO_frac_correc_aug[n] = gr_expj(-2 * M_PI * (cfo_frac) / m_number_of_bins * n);
            // Close the current scope block.
            }

            // Call volk_32fc_x2_multiply_32fc with arguments (&preamble_upchirps[0], samples, &CFO_frac_correc_aug[0], up_symb_to_use * m_number_of_bins).
            volk_32fc_x2_multiply_32fc(&preamble_upchirps[0], samples, &CFO_frac_correc_aug[0], up_symb_to_use * m_number_of_bins);

            // Return cfo_frac to the caller.
            return cfo_frac;
        // Close the current scope block.
        }
        // Define the l::estimate_CFO_frac_Bernier function.
        float frame_sync_impl::estimate_CFO_frac_Bernier(gr_complex *samples)
        // Open a new scope block.
        {
            // Execute statement std::vector<int> k0(up_symb_to_use).
            std::vector<int> k0(up_symb_to_use);
            // Declare float cfo_frac.
            float cfo_frac;
            // Execute statement std::vector<gr_complex> CFO_frac_correc_aug(up_symb_to_use * m_number_of_bins). (inline comment notes: /< CFO frac correction vector)
            std::vector<gr_complex> CFO_frac_correc_aug(up_symb_to_use * m_number_of_bins); ///< CFO frac correction vector
            // Execute statement std::vector<double> k0_mag(up_symb_to_use).
            std::vector<double> k0_mag(up_symb_to_use);
            // Execute statement std::vector<gr_complex> fft_val(up_symb_to_use * m_number_of_bins).
            std::vector<gr_complex> fft_val(up_symb_to_use * m_number_of_bins);

            // Execute statement std::vector<gr_complex> dechirped(m_number_of_bins).
            std::vector<gr_complex> dechirped(m_number_of_bins);
            // Assign kiss_fft_cpx *cx_in_cfo to new kiss_fft_cpx[m_number_of_bins].
            kiss_fft_cpx *cx_in_cfo = new kiss_fft_cpx[m_number_of_bins];
            // Assign kiss_fft_cpx *cx_out_cfo to new kiss_fft_cpx[m_number_of_bins].
            kiss_fft_cpx *cx_out_cfo = new kiss_fft_cpx[m_number_of_bins];
            // Execute statement std::vector<float> fft_mag_sq(m_number_of_bins).
            std::vector<float> fft_mag_sq(m_number_of_bins);
            // Iterate with loop parameters (size_t i = 0; i < m_number_of_bins; i++).
            for (size_t i = 0; i < m_number_of_bins; i++)
            // Open a new scope block.
            {
                // Assign fft_mag_sq[i] to 0.
                fft_mag_sq[i] = 0;
            // Close the current scope block.
            }
            // Assign kiss_fft_cfg cfg_cfo to kiss_fft_alloc(m_number_of_bins, 0, 0, 0).
            kiss_fft_cfg cfg_cfo = kiss_fft_alloc(m_number_of_bins, 0, 0, 0);
            // Iterate with loop parameters (int i = 0; i < up_symb_to_use; i++).
            for (int i = 0; i < up_symb_to_use; i++)
            // Open a new scope block.
            {
                // Dechirping
                // Call volk_32fc_x2_multiply_32fc with arguments (&dechirped[0], &samples[m_number_of_bins * i], &m_downchirp[0], m_number_of_bins).
                volk_32fc_x2_multiply_32fc(&dechirped[0], &samples[m_number_of_bins * i], &m_downchirp[0], m_number_of_bins);
                // prepare FFT
                // Iterate with loop parameters (uint32_t j = 0; j < m_number_of_bins; j++).
                for (uint32_t j = 0; j < m_number_of_bins; j++)
                // Open a new scope block.
                {
                    // Assign cx_in_cfo[j].r to dechirped[j].real().
                    cx_in_cfo[j].r = dechirped[j].real();
                    // Assign cx_in_cfo[j].i to dechirped[j].imag().
                    cx_in_cfo[j].i = dechirped[j].imag();
                // Close the current scope block.
                }
                // do the FFT
                // Call kiss_fft with arguments (cfg_cfo, cx_in_cfo, cx_out_cfo).
                kiss_fft(cfg_cfo, cx_in_cfo, cx_out_cfo);
                // Get magnitude

                // Iterate with loop parameters (uint32_t j = 0u; j < m_number_of_bins; j++).
                for (uint32_t j = 0u; j < m_number_of_bins; j++)
                // Open a new scope block.
                {
                    // Assign fft_mag_sq[j] to cx_out_cfo[j].r * cx_out_cfo[j].r + cx_out_cfo[j].i * cx_out_cfo[j].i.
                    fft_mag_sq[j] = cx_out_cfo[j].r * cx_out_cfo[j].r + cx_out_cfo[j].i * cx_out_cfo[j].i;
                    // Assign fft_val[j + i * m_number_of_bins] to gr_complex(cx_out_cfo[j].r, cx_out_cfo[j].i).
                    fft_val[j + i * m_number_of_bins] = gr_complex(cx_out_cfo[j].r, cx_out_cfo[j].i);
                // Close the current scope block.
                }
                // Assign k0[i] to std::distance(std::begin(fft_mag_sq), std::max_element(std::begin(fft_mag_sq), std::end(fft_mag_sq))).
                k0[i] = std::distance(std::begin(fft_mag_sq), std::max_element(std::begin(fft_mag_sq), std::end(fft_mag_sq)));

                // Assign k0_mag[i] to fft_mag_sq[k0[i]].
                k0_mag[i] = fft_mag_sq[k0[i]];
            // Close the current scope block.
            }
            // Call free with arguments (cfg_cfo).
            free(cfg_cfo);
            // Declare delete[] cx_in_cfo.
            delete[] cx_in_cfo;
            // Declare delete[] cx_out_cfo.
            delete[] cx_out_cfo;
            // get argmax
            // Assign int idx_max to k0[std::distance(std::begin(k0_mag), std::max_element(std::begin(k0_mag), std::end(k0_mag)))].
            int idx_max = k0[std::distance(std::begin(k0_mag), std::max_element(std::begin(k0_mag), std::end(k0_mag)))];
            // Execute statement gr_complex four_cum(0.0f, 0.0f).
            gr_complex four_cum(0.0f, 0.0f);
            // Iterate with loop parameters (int i = 0; i < up_symb_to_use - 1; i++).
            for (int i = 0; i < up_symb_to_use - 1; i++)
            // Open a new scope block.
            {
                // Assign four_cum + to fft_val[idx_max + m_number_of_bins * i] * std::conj(fft_val[idx_max + m_number_of_bins * (i + 1)]).
                four_cum += fft_val[idx_max + m_number_of_bins * i] * std::conj(fft_val[idx_max + m_number_of_bins * (i + 1)]);
            // Close the current scope block.
            }
            // Assign cfo_frac to -std::arg(four_cum) / 2 / M_PI.
            cfo_frac = -std::arg(four_cum) / 2 / M_PI;
            // Correct CFO in preamble
            // Iterate with loop parameters (uint32_t n = 0; n < up_symb_to_use * m_number_of_bins; n++).
            for (uint32_t n = 0; n < up_symb_to_use * m_number_of_bins; n++)
            // Open a new scope block.
            {
                // Assign CFO_frac_correc_aug[n] to gr_expj(-2 * M_PI * cfo_frac / m_number_of_bins * n).
                CFO_frac_correc_aug[n] = gr_expj(-2 * M_PI * cfo_frac / m_number_of_bins * n);
            // Close the current scope block.
            }
            // Call volk_32fc_x2_multiply_32fc with arguments (&preamble_upchirps[0], samples, &CFO_frac_correc_aug[0], up_symb_to_use * m_number_of_bins).
            volk_32fc_x2_multiply_32fc(&preamble_upchirps[0], samples, &CFO_frac_correc_aug[0], up_symb_to_use * m_number_of_bins);
            // Return cfo_frac to the caller.
            return cfo_frac;
        // Close the current scope block.
        }

        // Define the l::estimate_STO_frac function.
        float frame_sync_impl::estimate_STO_frac()
        // Open a new scope block.
        {
            // Declare int k0.
            int k0;
            // Start the body of a do-while loop.
            double Y_1, Y0, Y1, u, v, ka, wa, k_residual;
            // Assign float sto_frac to 0.
            float sto_frac = 0;

            // Execute statement std::vector<gr_complex> dechirped(m_number_of_bins).
            std::vector<gr_complex> dechirped(m_number_of_bins);
            // Assign kiss_fft_cpx *cx_in_sto to new kiss_fft_cpx[2 * m_number_of_bins].
            kiss_fft_cpx *cx_in_sto = new kiss_fft_cpx[2 * m_number_of_bins];
            // Assign kiss_fft_cpx *cx_out_sto to new kiss_fft_cpx[2 * m_number_of_bins].
            kiss_fft_cpx *cx_out_sto = new kiss_fft_cpx[2 * m_number_of_bins];

            // Execute statement std::vector<float> fft_mag_sq(2 * m_number_of_bins).
            std::vector<float> fft_mag_sq(2 * m_number_of_bins);
            // Iterate with loop parameters (size_t i = 0; i < 2 * m_number_of_bins; i++).
            for (size_t i = 0; i < 2 * m_number_of_bins; i++)
            // Open a new scope block.
            {
                // Assign fft_mag_sq[i] to 0.
                fft_mag_sq[i] = 0;
            // Close the current scope block.
            }
            // Assign kiss_fft_cfg cfg_sto to kiss_fft_alloc(2 * m_number_of_bins, 0, 0, 0).
            kiss_fft_cfg cfg_sto = kiss_fft_alloc(2 * m_number_of_bins, 0, 0, 0);

            // Iterate with loop parameters (int i = 0; i < up_symb_to_use; i++).
            for (int i = 0; i < up_symb_to_use; i++)
            // Open a new scope block.
            {
                // Dechirping
                // Call volk_32fc_x2_multiply_32fc with arguments (&dechirped[0], &preamble_upchirps[m_number_of_bins * i], &m_downchirp[0], m_number_of_bins).
                volk_32fc_x2_multiply_32fc(&dechirped[0], &preamble_upchirps[m_number_of_bins * i], &m_downchirp[0], m_number_of_bins);

                // prepare FFT
                // Iterate with loop parameters (uint32_t j = 0; j < 2 * m_number_of_bins; j++).
                for (uint32_t j = 0; j < 2 * m_number_of_bins; j++)
                // Open a new scope block.
                {
                    // Branch when condition (j < m_number_of_bins) evaluates to true.
                    if (j < m_number_of_bins)
                    // Open a new scope block.
                    {
                        // Assign cx_in_sto[j].r to dechirped[j].real().
                        cx_in_sto[j].r = dechirped[j].real();
                        // Assign cx_in_sto[j].i to dechirped[j].imag().
                        cx_in_sto[j].i = dechirped[j].imag();
                    // Close the current scope block.
                    }
                    // Handle the alternative branch when previous conditions fail.
                    else
                    // Open a new scope block. (inline comment notes: add padding)
                    { // add padding
                        // Assign cx_in_sto[j].r to 0.
                        cx_in_sto[j].r = 0;
                        // Assign cx_in_sto[j].i to 0.
                        cx_in_sto[j].i = 0;
                    // Close the current scope block.
                    }
                // Close the current scope block.
                }
                // do the FFT
                // Call kiss_fft with arguments (cfg_sto, cx_in_sto, cx_out_sto).
                kiss_fft(cfg_sto, cx_in_sto, cx_out_sto);
                // Get magnitude
                // Iterate with loop parameters (uint32_t j = 0u; j < 2 * m_number_of_bins; j++).
                for (uint32_t j = 0u; j < 2 * m_number_of_bins; j++)
                // Open a new scope block.
                {
                    // Assign fft_mag_sq[j] + to cx_out_sto[j].r * cx_out_sto[j].r + cx_out_sto[j].i * cx_out_sto[j].i.
                    fft_mag_sq[j] += cx_out_sto[j].r * cx_out_sto[j].r + cx_out_sto[j].i * cx_out_sto[j].i;
                // Close the current scope block.
                }
            // Close the current scope block.
            }
            // Call free with arguments (cfg_sto).
            free(cfg_sto);
            // Declare delete[] cx_in_sto.
            delete[] cx_in_sto;
            // Declare delete[] cx_out_sto.
            delete[] cx_out_sto;

            // get argmax here
            // Assign k0 to std::distance(std::begin(fft_mag_sq), std::max_element(std::begin(fft_mag_sq), std::end(fft_mag_sq))).
            k0 = std::distance(std::begin(fft_mag_sq), std::max_element(std::begin(fft_mag_sq), std::end(fft_mag_sq)));

            // get three spectral lines
            // Assign Y_1 to fft_mag_sq[mod(k0 - 1, 2 * m_number_of_bins)].
            Y_1 = fft_mag_sq[mod(k0 - 1, 2 * m_number_of_bins)];
            // Assign Y0 to fft_mag_sq[k0].
            Y0 = fft_mag_sq[k0];
            // Assign Y1 to fft_mag_sq[mod(k0 + 1, 2 * m_number_of_bins)].
            Y1 = fft_mag_sq[mod(k0 + 1, 2 * m_number_of_bins)];

            // set constant coeff
            // Assign u to 64 * m_number_of_bins / 406.5506497. (inline comment notes: from Cui yang (eq.15))
            u = 64 * m_number_of_bins / 406.5506497; // from Cui yang (eq.15)
            // Assign v to u * 2.4674.
            v = u * 2.4674;
            // RCTSL
            // Assign wa to (Y1 - Y_1) / (u * (Y1 + Y_1) + v * Y0).
            wa = (Y1 - Y_1) / (u * (Y1 + Y_1) + v * Y0);
            // Assign ka to wa * m_number_of_bins / M_PI.
            ka = wa * m_number_of_bins / M_PI;
            // Assign k_residual to fmod((k0 + ka) / 2, 1).
            k_residual = fmod((k0 + ka) / 2, 1);
            // Assign sto_frac to k_residual - (k_residual > 0.5 ? 1 : 0).
            sto_frac = k_residual - (k_residual > 0.5 ? 1 : 0);

            // Return sto_frac to the caller.
            return sto_frac;
        // Close the current scope block.
        }

        // Define the l::get_symbol_val function.
        uint32_t frame_sync_impl::get_symbol_val(const gr_complex *samples, gr_complex *ref_chirp)
        // Open a new scope block.
        {
            // Start the body of a do-while loop.
            double sig_en = 0;
            // Execute statement std::vector<float> fft_mag(m_number_of_bins).
            std::vector<float> fft_mag(m_number_of_bins);
            // Execute statement volk::vector<gr_complex> dechirped(m_number_of_bins).
            volk::vector<gr_complex> dechirped(m_number_of_bins);

            // Multiply with ideal downchirp
            // Call volk_32fc_x2_multiply_32fc with arguments (&dechirped[0], samples, ref_chirp, m_number_of_bins).
            volk_32fc_x2_multiply_32fc(&dechirped[0], samples, ref_chirp, m_number_of_bins);

            // Iterate with loop parameters (uint32_t i = 0; i < m_number_of_bins; i++).
            for (uint32_t i = 0; i < m_number_of_bins; i++)
            // Open a new scope block.
            {
                // Assign cx_in[i].r to dechirped[i].real().
                cx_in[i].r = dechirped[i].real();
                // Assign cx_in[i].i to dechirped[i].imag().
                cx_in[i].i = dechirped[i].imag();
            // Close the current scope block.
            }
            // do the FFT
            // Call kiss_fft with arguments (m_kiss_fft_cfg, cx_in, cx_out).
            kiss_fft(m_kiss_fft_cfg, cx_in, cx_out);

            // Get magnitude
            // Iterate with loop parameters (uint32_t i = 0u; i < m_number_of_bins; i++).
            for (uint32_t i = 0u; i < m_number_of_bins; i++)
            // Open a new scope block.
            {
                // Assign fft_mag[i] to cx_out[i].r * cx_out[i].r + cx_out[i].i * cx_out[i].i.
                fft_mag[i] = cx_out[i].r * cx_out[i].r + cx_out[i].i * cx_out[i].i;
                // Assign sig_en + to fft_mag[i].
                sig_en += fft_mag[i];
            // Close the current scope block.
            }
            // Return argmax here

            // Return sig_en ? (std::distance(std::begin(fft_mag), std::max_element(std::begin(fft_mag), std::end(fft_mag)))) : -1 to the caller.
            return sig_en ? (std::distance(std::begin(fft_mag), std::max_element(std::begin(fft_mag), std::end(fft_mag)))) : -1;
        // Close the current scope block.
        }

        // Define the l::determine_energy function.
        float frame_sync_impl::determine_energy(const gr_complex *samples, int length = 1)
        // Open a new scope block.
        {
            // Execute statement volk::vector<float> magsq_chirp(m_number_of_bins * length).
            volk::vector<float> magsq_chirp(m_number_of_bins * length);
            // Assign float energy_chirp to 0.
            float energy_chirp = 0;
            // Call volk_32fc_magnitude_squared_32f with arguments (&magsq_chirp[0], samples, m_number_of_bins * length).
            volk_32fc_magnitude_squared_32f(&magsq_chirp[0], samples, m_number_of_bins * length);
            // Call volk_32f_accumulator_s32f with arguments (&energy_chirp, &magsq_chirp[0], m_number_of_bins * length).
            volk_32f_accumulator_s32f(&energy_chirp, &magsq_chirp[0], m_number_of_bins * length);
            // Return energy_chirp / m_number_of_bins / length to the caller.
            return energy_chirp / m_number_of_bins / length;
        // Close the current scope block.
        }
        // Define the l::determine_snr function.
        float frame_sync_impl::determine_snr(const gr_complex *samples)
        // Open a new scope block.
        {
            // Start the body of a do-while loop.
            double tot_en = 0;
            // Execute statement std::vector<float> fft_mag(m_number_of_bins).
            std::vector<float> fft_mag(m_number_of_bins);
            // Execute statement std::vector<gr_complex> dechirped(m_number_of_bins).
            std::vector<gr_complex> dechirped(m_number_of_bins);

            // Assign kiss_fft_cfg cfg to kiss_fft_alloc(m_number_of_bins, 0, 0, 0).
            kiss_fft_cfg cfg = kiss_fft_alloc(m_number_of_bins, 0, 0, 0);

            // Multiply with ideal downchirp
            // Call volk_32fc_x2_multiply_32fc with arguments (&dechirped[0], samples, &m_downchirp[0], m_number_of_bins).
            volk_32fc_x2_multiply_32fc(&dechirped[0], samples, &m_downchirp[0], m_number_of_bins);

            // Iterate with loop parameters (uint32_t i = 0; i < m_number_of_bins; i++).
            for (uint32_t i = 0; i < m_number_of_bins; i++)
            // Open a new scope block.
            {
                // Assign cx_in[i].r to dechirped[i].real().
                cx_in[i].r = dechirped[i].real();
                // Assign cx_in[i].i to dechirped[i].imag().
                cx_in[i].i = dechirped[i].imag();
            // Close the current scope block.
            }
            // do the FFT
            // Call kiss_fft with arguments (cfg, cx_in, cx_out).
            kiss_fft(cfg, cx_in, cx_out);

            // Get magnitude
            // Iterate with loop parameters (uint32_t i = 0u; i < m_number_of_bins; i++).
            for (uint32_t i = 0u; i < m_number_of_bins; i++)
            // Open a new scope block.
            {
                // Assign fft_mag[i] to cx_out[i].r * cx_out[i].r + cx_out[i].i * cx_out[i].i.
                fft_mag[i] = cx_out[i].r * cx_out[i].r + cx_out[i].i * cx_out[i].i;
                // Assign tot_en + to fft_mag[i].
                tot_en += fft_mag[i];
            // Close the current scope block.
            }
            // Call free with arguments (cfg).
            free(cfg);

            // Assign int max_idx to std::distance(std::begin(fft_mag), std::max_element(std::begin(fft_mag), std::end(fft_mag))).
            int max_idx = std::distance(std::begin(fft_mag), std::max_element(std::begin(fft_mag), std::end(fft_mag)));
            // Assign float sig_en to fft_mag[max_idx].
            float sig_en = fft_mag[max_idx];
            // Return 10 * log10(sig_en / (tot_en - sig_en)) to the caller.
            return 10 * log10(sig_en / (tot_en - sig_en));
        // Close the current scope block.
        }

        // Define the l::noise_est_handler function.
        void frame_sync_impl::noise_est_handler(pmt::pmt_t noise_est)
        // Open a new scope block.
        {
            // Assign m_noise_est to pmt::to_double(noise_est).
            m_noise_est = pmt::to_double(noise_est);
        // Close the current scope block.
        }
        // Define the l::frame_info_handler function.
        void frame_sync_impl::frame_info_handler(pmt::pmt_t frame_info)
        // Open a new scope block.
        {
            // Assign pmt::pmt_t err to pmt::string_to_symbol("error").
            pmt::pmt_t err = pmt::string_to_symbol("error");

            // Assign m_cr to pmt::to_long(pmt::dict_ref(frame_info, pmt::string_to_symbol("cr"), err)).
            m_cr = pmt::to_long(pmt::dict_ref(frame_info, pmt::string_to_symbol("cr"), err));
            // Assign m_pay_len to pmt::to_double(pmt::dict_ref(frame_info, pmt::string_to_symbol("pay_len"), err)).
            m_pay_len = pmt::to_double(pmt::dict_ref(frame_info, pmt::string_to_symbol("pay_len"), err));
            // Assign m_has_crc to pmt::to_long(pmt::dict_ref(frame_info, pmt::string_to_symbol("crc"), err)).
            m_has_crc = pmt::to_long(pmt::dict_ref(frame_info, pmt::string_to_symbol("crc"), err));
            // Assign uint8_t ldro_mode to pmt::to_long(pmt::dict_ref(frame_info, pmt::string_to_symbol("ldro_mode"), err)).
            uint8_t ldro_mode = pmt::to_long(pmt::dict_ref(frame_info, pmt::string_to_symbol("ldro_mode"), err));
            // Assign m_invalid_header to pmt::to_double(pmt::dict_ref(frame_info, pmt::string_to_symbol("err"), err)).
            m_invalid_header = pmt::to_double(pmt::dict_ref(frame_info, pmt::string_to_symbol("err"), err));

            // Branch when condition (m_invalid_header) evaluates to true.
            if (m_invalid_header)
            // Open a new scope block.
            {
                // Assign m_state to DETECT.
                m_state = DETECT;
                // Assign symbol_cnt to 1.
                symbol_cnt = 1;
                // Assign k_hat to 0.
                k_hat = 0;
                // Assign m_sto_frac to 0.
                m_sto_frac = 0;
                // Assign m_symb_numb to 0.
                m_symb_numb = 0;
            // Close the current scope block.
            }
            // Handle the alternative branch when previous conditions fail.
            else
            // Open a new scope block.
            {
                // Branch when condition (ldro_mode == AUTO) evaluates to true.
                if (ldro_mode == AUTO)
                    // Assign m_ldro to (float)(1u << m_sf) * 1e3 / m_bw > LDRO_MAX_DURATION_MS.
                    m_ldro = (float)(1u << m_sf) * 1e3 / m_bw > LDRO_MAX_DURATION_MS;
                // Handle the alternative branch when previous conditions fail.
                else
                    // Assign m_ldro to ldro_mode.
                    m_ldro = ldro_mode;

                // Assign m_symb_numb to 8 + ceil((double)(2 * m_pay_len - m_sf + 2 + !m_impl_head * 5 + m_has_crc * 4) / (m_sf - 2 * m_ldro)) * (4 + m_cr).
                m_symb_numb = 8 + ceil((double)(2 * m_pay_len - m_sf + 2 + !m_impl_head * 5 + m_has_crc * 4) / (m_sf - 2 * m_ldro)) * (4 + m_cr);
                // Assign m_received_head to true.
                m_received_head = true;
                // Assign frame_info to pmt::dict_add(frame_info, pmt::intern("is_header"), pmt::from_bool(false)).
                frame_info = pmt::dict_add(frame_info, pmt::intern("is_header"), pmt::from_bool(false));
                // Assign frame_info to pmt::dict_add(frame_info, pmt::intern("symb_numb"), pmt::from_long(m_symb_numb)).
                frame_info = pmt::dict_add(frame_info, pmt::intern("symb_numb"), pmt::from_long(m_symb_numb));
                // Assign frame_info to pmt::dict_delete(frame_info, pmt::intern("ldro_mode")).
                frame_info = pmt::dict_delete(frame_info, pmt::intern("ldro_mode"));

                // Assign frame_info to pmt::dict_add(frame_info, pmt::intern("ldro"), pmt::from_bool(m_ldro)).
                frame_info = pmt::dict_add(frame_info, pmt::intern("ldro"), pmt::from_bool(m_ldro));
                // Call add_item_tag with arguments (0, nitems_written(0), pmt::string_to_symbol("frame_info"), frame_info).
                add_item_tag(0, nitems_written(0), pmt::string_to_symbol("frame_info"), frame_info);
            // Close the current scope block.
            }
        // Close the current scope block.
        }

        // Define the l::set_sf function.
        void frame_sync_impl::set_sf(int sf)
        // Open a new scope block.
        {
            // Assign m_sf to sf.
            m_sf = sf;
            // Assign m_number_of_bins to (uint32_t)(1u << m_sf).
            m_number_of_bins = (uint32_t)(1u << m_sf);
            // Assign m_samples_per_symbol to m_number_of_bins * m_os_factor.
            m_samples_per_symbol = m_number_of_bins * m_os_factor;
            // Resize additional_symbol_samp to 2 * m_samples_per_symbol.
            additional_symbol_samp.resize(2 * m_samples_per_symbol);
            // Resize m_upchirp to m_number_of_bins.
            m_upchirp.resize(m_number_of_bins);
            // Resize m_downchirp to m_number_of_bins.
            m_downchirp.resize(m_number_of_bins);
            // Resize preamble_upchirps to m_preamb_len * m_number_of_bins.
            preamble_upchirps.resize(m_preamb_len * m_number_of_bins);
            // Resize preamble_raw_up to (m_preamb_len + 3) * m_samples_per_symbol.
            preamble_raw_up.resize((m_preamb_len + 3) * m_samples_per_symbol);
            // Resize CFO_frac_correc to m_number_of_bins.
            CFO_frac_correc.resize(m_number_of_bins);
            // Resize CFO_SFO_frac_correc to m_number_of_bins.
            CFO_SFO_frac_correc.resize(m_number_of_bins);
            // Resize symb_corr to m_number_of_bins.
            symb_corr.resize(m_number_of_bins);
            // Resize in_down to m_number_of_bins.
            in_down.resize(m_number_of_bins);
            // Resize preamble_raw to m_preamb_len * m_number_of_bins.
            preamble_raw.resize(m_preamb_len * m_number_of_bins);
            // Resize net_id_samp to m_samples_per_symbol * 2.5. (inline comment notes: we should be able to move up to one quarter of symbol in each direction)
            net_id_samp.resize(m_samples_per_symbol * 2.5); // we should be able to move up to one quarter of symbol in each direction
            // Call build_ref_chirps with arguments (&m_upchirp[0], &m_downchirp[0], m_sf).
            build_ref_chirps(&m_upchirp[0], &m_downchirp[0], m_sf);

            // Assign cx_in to new kiss_fft_cpx[m_number_of_bins].
            cx_in = new kiss_fft_cpx[m_number_of_bins];
            // Assign cx_out to new kiss_fft_cpx[m_number_of_bins].
            cx_out = new kiss_fft_cpx[m_number_of_bins];
            // Call set_output_multiple with arguments (m_number_of_bins).
            set_output_multiple(m_number_of_bins);
        // Close the current scope block.
        }

        // Specify parameter or initializer int frame_sync_impl::general_work(int noutput_items.
        int frame_sync_impl::general_work(int noutput_items,
                                          // Specify parameter or initializer gr_vector_int &ninput_items.
                                          gr_vector_int &ninput_items,
                                          // Specify parameter or initializer gr_vector_const_void_star &input_items.
                                          gr_vector_const_void_star &input_items,
                                          // Execute gr_vector_void_star &output_items).
                                          gr_vector_void_star &output_items)
        // Open a new scope block.
        {
            // Assign const gr_complex *in to (const gr_complex *)input_items[0].
            const gr_complex *in = (const gr_complex *)input_items[0];
            // Assign gr_complex *out to (gr_complex *)output_items[0].
            gr_complex *out = (gr_complex *)output_items[0];
            // Assign int items_to_output to 0.
            int items_to_output = 0;

            // check if there is enough space in the output buffer
            // Branch when condition ((uint32_t)noutput_items < m_number_of_bins) evaluates to true.
            if ((uint32_t)noutput_items < m_number_of_bins)
            // Open a new scope block.
            {
                // Return 0 to the caller.
                return 0;
            // Close the current scope block.
            }

            // Assign float *sync_log_out to NULL.
            float *sync_log_out = NULL;
            // Branch when condition (output_items.size() == 2) evaluates to true.
            if (output_items.size() == 2)
            // Open a new scope block.
            {
                // Assign sync_log_out to (float *)output_items[1].
                sync_log_out = (float *)output_items[1];
                // Assign m_should_log to true.
                m_should_log = true;
            // Close the current scope block.
            }
            // Handle the alternative branch when previous conditions fail.
            else
                // Assign m_should_log to false.
                m_should_log = false;
            // Assign int nitems_to_process to ninput_items[0].
            int nitems_to_process = ninput_items[0];

            // Declare std::vector<tag_t> tags.
            std::vector<tag_t> tags;
            // Call get_tags_in_window with arguments (tags, 0, 0, ninput_items[0], pmt::string_to_symbol("new_frame")).
            get_tags_in_window(tags, 0, 0, ninput_items[0], pmt::string_to_symbol("new_frame"));
            // Branch when condition (tags.size()) evaluates to true.
            if (tags.size())
            // Open a new scope block.
            {
                // Branch when condition (tags[0].offset != nitems_read(0)) evaluates to true.
                if (tags[0].offset != nitems_read(0))
                    // Assign nitems_to_process to tags[0].offset - nitems_read(0). (inline comment notes: only use symbol until the next frame begin (SF might change))
                    nitems_to_process = tags[0].offset - nitems_read(0); // only use symbol until the next frame begin (SF might change)

                // Handle the alternative branch when previous conditions fail.
                else
                // Open a new scope block.
                {
                    // Branch when condition (tags.size() >= 2) evaluates to true.
                    if (tags.size() >= 2)
                        // Assign nitems_to_process to tags[1].offset - tags[0].offset.
                        nitems_to_process = tags[1].offset - tags[0].offset;

                    // Assign pmt::pmt_t err to pmt::string_to_symbol("error").
                    pmt::pmt_t err = pmt::string_to_symbol("error");

                    // Assign int sf to pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("sf"), err)).
                    int sf = pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("sf"), err));
                    // Call set_sf with arguments (sf).
                    set_sf(sf);

                    // std::cout<<"\nhamming_cr "<<tags[0].offset<<" - cr: "<<(int)m_cr<<"\n";
                // Close the current scope block.
                }
            // Close the current scope block.
            }

            // downsampling
            // Iterate with loop parameters (uint32_t ii = 0; ii < m_number_of_bins; ii++).
            for (uint32_t ii = 0; ii < m_number_of_bins; ii++)
                // Assign in_down[ii] to in[(int)(m_os_factor / 2 + m_os_factor * ii - my_roundf(m_sto_frac * m_os_factor))].
                in_down[ii] = in[(int)(m_os_factor / 2 + m_os_factor * ii - my_roundf(m_sto_frac * m_os_factor))];

            // Select behavior based on (m_state).
            switch (m_state)
            // Open a new scope block.
            {
            // Handle switch label case DETECT.
            case DETECT:
            // Open a new scope block.
            {
                // Assign bin_idx_new to get_symbol_val(&in_down[0], &m_downchirp[0]).
                bin_idx_new = get_symbol_val(&in_down[0], &m_downchirp[0]);

                // Branch when condition (abs(mod(abs(bin_idx_new - bin_idx) + 1, m_number_of_bins) - 1) <= 1 && bin_idx_new != -1) evaluates to true. (inline comment notes: look for consecutive reference upchirps(with a margin of ±1))
                if (abs(mod(abs(bin_idx_new - bin_idx) + 1, m_number_of_bins) - 1) <= 1 && bin_idx_new != -1) // look for consecutive reference upchirps(with a margin of ±1)
                // Open a new scope block.
                {
                    // Branch when condition (symbol_cnt == 1 && bin_idx != -1) evaluates to true.
                    if (symbol_cnt == 1 && bin_idx != -1)
                        // Assign preamb_up_vals[0] to bin_idx.
                        preamb_up_vals[0] = bin_idx;

                    // Assign preamb_up_vals[symbol_cnt] to bin_idx_new.
                    preamb_up_vals[symbol_cnt] = bin_idx_new;
                    // Call memcpy with arguments (&preamble_raw[m_number_of_bins * symbol_cnt], &in_down[0], m_number_of_bins * sizeof(gr_complex)).
                    memcpy(&preamble_raw[m_number_of_bins * symbol_cnt], &in_down[0], m_number_of_bins * sizeof(gr_complex));
                    // Call memcpy with arguments (&preamble_raw_up[m_samples_per_symbol * symbol_cnt], &in[(int)(m_os_factor / 2)], m_samples_per_symbol * sizeof(gr_complex)).
                    memcpy(&preamble_raw_up[m_samples_per_symbol * symbol_cnt], &in[(int)(m_os_factor / 2)], m_samples_per_symbol * sizeof(gr_complex));

                    // Increment symbol_cnt.
                    symbol_cnt++;
                // Close the current scope block.
                }
                // Handle the alternative branch when previous conditions fail.
                else
                // Open a new scope block.
                {
                    // Call memcpy with arguments (&preamble_raw[0], &in_down[0], m_number_of_bins * sizeof(gr_complex)).
                    memcpy(&preamble_raw[0], &in_down[0], m_number_of_bins * sizeof(gr_complex));
                    // Call memcpy with arguments (&preamble_raw_up[0], &in[(int)(m_os_factor / 2)], m_samples_per_symbol * sizeof(gr_complex)).
                    memcpy(&preamble_raw_up[0], &in[(int)(m_os_factor / 2)], m_samples_per_symbol * sizeof(gr_complex));

                    // Assign symbol_cnt to 1.
                    symbol_cnt = 1;
                // Close the current scope block.
                }
                // Assign bin_idx to bin_idx_new.
                bin_idx = bin_idx_new;
                // Branch when condition (symbol_cnt == (int)(m_n_up_req)) evaluates to true.
                if (symbol_cnt == (int)(m_n_up_req))
                // Open a new scope block.
                {
                    // Assign additional_upchirps to 0.
                    additional_upchirps = 0;
                    // Assign m_state to SYNC.
                    m_state = SYNC;
                    // Assign symbol_cnt to 0.
                    symbol_cnt = 0;
                    // Assign cfo_frac_sto_frac_est to false.
                    cfo_frac_sto_frac_est = false;
                    // Assign k_hat to most_frequent(&preamb_up_vals[0], preamb_up_vals.size()).
                    k_hat = most_frequent(&preamb_up_vals[0], preamb_up_vals.size());
                    // Call memcpy with arguments (&net_id_samp[0], &in[int(0.75 * m_samples_per_symbol - k_hat * m_os_factor)], sizeof(gr_complex) * 0.25 * m_samples_per_symbol).
                    memcpy(&net_id_samp[0], &in[int(0.75 * m_samples_per_symbol - k_hat * m_os_factor)], sizeof(gr_complex) * 0.25 * m_samples_per_symbol);

                    // perform the coarse synchronization
                    // Assign items_to_consume to m_os_factor * ((int)(m_number_of_bins - k_hat)).
                    items_to_consume = m_os_factor * ((int)(m_number_of_bins - k_hat));
                // Close the current scope block.
                }
                // Handle the alternative branch when previous conditions fail.
                else
                    // Assign items_to_consume to m_samples_per_symbol.
                    items_to_consume = m_samples_per_symbol;
                // Assign items_to_output to 0.
                items_to_output = 0;
                // Exit the nearest enclosing loop or switch.
                break;
            // Close the current scope block.
            }
            // Handle switch label case SYNC.
            case SYNC:
            // Open a new scope block.
            {
                // Assign items_to_output to 0.
                items_to_output = 0;
                // Branch when condition (!cfo_frac_sto_frac_est) evaluates to true.
                if (!cfo_frac_sto_frac_est)
                // Open a new scope block.
                {
                    // Assign m_cfo_frac to estimate_CFO_frac_Bernier(&preamble_raw[m_number_of_bins - k_hat]).
                    m_cfo_frac = estimate_CFO_frac_Bernier(&preamble_raw[m_number_of_bins - k_hat]);
                    // Assign m_sto_frac to estimate_STO_frac().
                    m_sto_frac = estimate_STO_frac();
                    // create correction vector
                    // Iterate with loop parameters (uint32_t n = 0; n < m_number_of_bins; n++).
                    for (uint32_t n = 0; n < m_number_of_bins; n++)
                    // Open a new scope block.
                    {
                        // Assign CFO_frac_correc[n] to gr_expj(-2 * M_PI * m_cfo_frac / m_number_of_bins * n).
                        CFO_frac_correc[n] = gr_expj(-2 * M_PI * m_cfo_frac / m_number_of_bins * n);
                    // Close the current scope block.
                    }
                    // Assign cfo_frac_sto_frac_est to true.
                    cfo_frac_sto_frac_est = true;
                // Close the current scope block.
                }
                // Assign items_to_consume to m_samples_per_symbol.
                items_to_consume = m_samples_per_symbol;
                // apply cfo correction
                // Call volk_32fc_x2_multiply_32fc with arguments (&symb_corr[0], &in_down[0], &CFO_frac_correc[0], m_number_of_bins).
                volk_32fc_x2_multiply_32fc(&symb_corr[0], &in_down[0], &CFO_frac_correc[0], m_number_of_bins);

                // Assign bin_idx to get_symbol_val(&symb_corr[0], &m_downchirp[0]).
                bin_idx = get_symbol_val(&symb_corr[0], &m_downchirp[0]);
                // Select behavior based on (symbol_cnt).
                switch (symbol_cnt)
                // Open a new scope block.
                {
                // Handle switch label case NET_ID1.
                case NET_ID1:
                // Open a new scope block.
                {
                    // Branch when condition (bin_idx == 0 || bin_idx == 1 || (uint32_t)bin_idx == m_number_of_bins - 1) evaluates to true.
                    if (bin_idx == 0 || bin_idx == 1 || (uint32_t)bin_idx == m_number_of_bins - 1)
                    // Open a new scope block. (inline comment notes: look for additional upchirps. Won't work if network identifier 1 equals 2^sf-1, 0 or 1!)
                    { // look for additional upchirps. Won't work if network identifier 1 equals 2^sf-1, 0 or 1!
                        // Call memcpy with arguments (&net_id_samp[0], &in[(int)0.75 * m_samples_per_symbol], sizeof(gr_complex) * 0.25 * m_samples_per_symbol).
                        memcpy(&net_id_samp[0], &in[(int)0.75 * m_samples_per_symbol], sizeof(gr_complex) * 0.25 * m_samples_per_symbol);
                        // Branch when condition (additional_upchirps >= 3) evaluates to true.
                        if (additional_upchirps >= 3)
                        // Open a new scope block.
                        {
                            // Call std::rotate with arguments (preamble_raw_up.begin(), preamble_raw_up.begin() + m_samples_per_symbol, preamble_raw_up.end()).
                            std::rotate(preamble_raw_up.begin(), preamble_raw_up.begin() + m_samples_per_symbol, preamble_raw_up.end());
                            // Call memcpy with arguments (&preamble_raw_up[m_samples_per_symbol * (m_n_up_req + 3)], &in[(int)(m_os_factor / 2) + k_hat * m_os_factor], m_samples_per_symbol * sizeof(gr_complex)).
                            memcpy(&preamble_raw_up[m_samples_per_symbol * (m_n_up_req + 3)], &in[(int)(m_os_factor / 2) + k_hat * m_os_factor], m_samples_per_symbol * sizeof(gr_complex));
                        // Close the current scope block.
                        }
                        // Handle the alternative branch when previous conditions fail.
                        else
                        // Open a new scope block.
                        {
                            // Call memcpy with arguments (&preamble_raw_up[m_samples_per_symbol * (m_n_up_req + additional_upchirps)], &in[(int)(m_os_factor / 2) + k_hat * m_os_factor], m_samples_per_symbol * sizeof(gr_complex)).
                            memcpy(&preamble_raw_up[m_samples_per_symbol * (m_n_up_req + additional_upchirps)], &in[(int)(m_os_factor / 2) + k_hat * m_os_factor], m_samples_per_symbol * sizeof(gr_complex));
                            // Increment additional_upchirps.
                            additional_upchirps++;
                        // Close the current scope block.
                        }
                    // Close the current scope block.
                    }
                    // Handle the alternative branch when previous conditions fail.
                    else
                    // Open a new scope block. (inline comment notes: network identifier 1 correct or off by one)
                    { // network identifier 1 correct or off by one
                        // Assign symbol_cnt to NET_ID2.
                        symbol_cnt = NET_ID2;
                        // Call memcpy with arguments (&net_id_samp[0.25 * m_samples_per_symbol], &in[0], sizeof(gr_complex) * m_samples_per_symbol).
                        memcpy(&net_id_samp[0.25 * m_samples_per_symbol], &in[0], sizeof(gr_complex) * m_samples_per_symbol);
                        // Assign net_ids[0] to bin_idx.
                        net_ids[0] = bin_idx;
                    // Close the current scope block.
                    }
                    // Exit the nearest enclosing loop or switch.
                    break;
                // Close the current scope block.
                }
                // Handle switch label case NET_ID2.
                case NET_ID2:
                // Open a new scope block.
                {

                    // Assign symbol_cnt to DOWNCHIRP1.
                    symbol_cnt = DOWNCHIRP1;
                    // Call memcpy with arguments (&net_id_samp[1.25 * m_samples_per_symbol], &in[0], sizeof(gr_complex) * (m_number_of_bins + 1) * m_os_factor).
                    memcpy(&net_id_samp[1.25 * m_samples_per_symbol], &in[0], sizeof(gr_complex) * (m_number_of_bins + 1) * m_os_factor);
                    // Assign net_ids[1] to bin_idx.
                    net_ids[1] = bin_idx;

                    // Exit the nearest enclosing loop or switch.
                    break;
                // Close the current scope block.
                }
                // Handle switch label case DOWNCHIRP1.
                case DOWNCHIRP1:
                // Open a new scope block.
                {
                    // Call memcpy with arguments (&net_id_samp[2.25 * m_samples_per_symbol], &in[0], sizeof(gr_complex) * 0.25 * m_samples_per_symbol).
                    memcpy(&net_id_samp[2.25 * m_samples_per_symbol], &in[0], sizeof(gr_complex) * 0.25 * m_samples_per_symbol);
                    // Assign symbol_cnt to DOWNCHIRP2.
                    symbol_cnt = DOWNCHIRP2;
                    // Exit the nearest enclosing loop or switch.
                    break;
                // Close the current scope block.
                }
                // Handle switch label case DOWNCHIRP2.
                case DOWNCHIRP2:
                // Open a new scope block.
                {
                    // Start the body of a do-while loop.
                    down_val = get_symbol_val(&symb_corr[0], &m_upchirp[0]);
                    // Call memcpy with arguments (&additional_symbol_samp[0], &in[0], sizeof(gr_complex) * m_samples_per_symbol).
                    memcpy(&additional_symbol_samp[0], &in[0], sizeof(gr_complex) * m_samples_per_symbol);
                    // Assign symbol_cnt to QUARTER_DOWN.
                    symbol_cnt = QUARTER_DOWN;
                    // Exit the nearest enclosing loop or switch.
                    break;
                // Close the current scope block.
                }
                // Handle switch label case QUARTER_DOWN.
                case QUARTER_DOWN:
                // Open a new scope block.
                {
                    // Call memcpy with arguments (&additional_symbol_samp[m_samples_per_symbol], &in[0], sizeof(gr_complex) * m_samples_per_symbol).
                    memcpy(&additional_symbol_samp[m_samples_per_symbol], &in[0], sizeof(gr_complex) * m_samples_per_symbol);
                    // Branch when condition ((uint32_t)down_val < m_number_of_bins / 2) evaluates to true.
                    if ((uint32_t)down_val < m_number_of_bins / 2)
                    // Open a new scope block.
                    {
                        // Assign m_cfo_int to floor(down_val / 2).
                        m_cfo_int = floor(down_val / 2);
                    // Close the current scope block.
                    }
                    // Handle the alternative branch when previous conditions fail.
                    else
                    // Open a new scope block.
                    {
                        // Assign m_cfo_int to floor(double(down_val - (int)m_number_of_bins) / 2).
                        m_cfo_int = floor(double(down_val - (int)m_number_of_bins) / 2);
                    // Close the current scope block.
                    }

                    // correct STOint and CFOint in the preamble upchirps
                    // Call std::rotate with arguments (preamble_upchirps.begin(), preamble_upchirps.begin() + mod(m_cfo_int, m_number_of_bins), preamble_upchirps.end()).
                    std::rotate(preamble_upchirps.begin(), preamble_upchirps.begin() + mod(m_cfo_int, m_number_of_bins), preamble_upchirps.end());

                    // Declare std::vector<gr_complex> CFO_int_correc.
                    std::vector<gr_complex> CFO_int_correc;
                    // Resize CFO_int_correc to (m_n_up_req + additional_upchirps) * m_number_of_bins.
                    CFO_int_correc.resize((m_n_up_req + additional_upchirps) * m_number_of_bins);
                    // Iterate with loop parameters (uint32_t n = 0; n < (m_n_up_req + additional_upchirps) * m_number_of_bins; n++).
                    for (uint32_t n = 0; n < (m_n_up_req + additional_upchirps) * m_number_of_bins; n++)
                    // Open a new scope block.
                    {
                        // Assign CFO_int_correc[n] to gr_expj(-2 * M_PI * (m_cfo_int) / m_number_of_bins * n).
                        CFO_int_correc[n] = gr_expj(-2 * M_PI * (m_cfo_int) / m_number_of_bins * n);
                    // Close the current scope block.
                    }

                    // Call volk_32fc_x2_multiply_32fc with arguments (&preamble_upchirps[0], &preamble_upchirps[0], &CFO_int_correc[0], up_symb_to_use * m_number_of_bins).
                    volk_32fc_x2_multiply_32fc(&preamble_upchirps[0], &preamble_upchirps[0], &CFO_int_correc[0], up_symb_to_use * m_number_of_bins);

                    // correct SFO in the preamble upchirps

                    // Assign sfo_hat to float((m_cfo_int + m_cfo_frac) * m_bw) / m_center_freq.
                    sfo_hat = float((m_cfo_int + m_cfo_frac) * m_bw) / m_center_freq;
                    // Start the body of a do-while loop.
                    double clk_off = sfo_hat / m_number_of_bins;
                    // Start the body of a do-while loop.
                    double fs = m_bw;
                    // Start the body of a do-while loop.
                    double fs_p = m_bw * (1 - clk_off);
                    // Assign int N to m_number_of_bins.
                    int N = m_number_of_bins;
                    // Declare std::vector<gr_complex> sfo_corr_vect.
                    std::vector<gr_complex> sfo_corr_vect;
                    // Resize sfo_corr_vect to (m_n_up_req + additional_upchirps) * m_number_of_bins, 0.
                    sfo_corr_vect.resize((m_n_up_req + additional_upchirps) * m_number_of_bins, 0);
                    // Iterate with loop parameters (uint32_t n = 0; n < (m_n_up_req + additional_upchirps) * m_number_of_bins; n++).
                    for (uint32_t n = 0; n < (m_n_up_req + additional_upchirps) * m_number_of_bins; n++)
                    // Open a new scope block.
                    {
                        // Assign sfo_corr_vect[n] to gr_expj(-2 * M_PI * (pow(mod(n, N), 2) / 2 / N * (m_bw / fs_p * m_bw / fs_p - m_bw / fs * m_bw / fs) + (std::floor((float)n / N) * (m_bw / fs_p * m_bw / fs_p - m_bw / fs_p) + m_bw / 2 * (1 / fs - 1 / fs_p)) * mod(n, N))).
                        sfo_corr_vect[n] = gr_expj(-2 * M_PI * (pow(mod(n, N), 2) / 2 / N * (m_bw / fs_p * m_bw / fs_p - m_bw / fs * m_bw / fs) + (std::floor((float)n / N) * (m_bw / fs_p * m_bw / fs_p - m_bw / fs_p) + m_bw / 2 * (1 / fs - 1 / fs_p)) * mod(n, N)));
                    // Close the current scope block.
                    }

                    // Call volk_32fc_x2_multiply_32fc with arguments (&preamble_upchirps[0], &preamble_upchirps[0], &sfo_corr_vect[0], up_symb_to_use * m_number_of_bins).
                    volk_32fc_x2_multiply_32fc(&preamble_upchirps[0], &preamble_upchirps[0], &sfo_corr_vect[0], up_symb_to_use * m_number_of_bins);

                    // Assign float tmp_sto_frac to estimate_STO_frac(). (inline comment notes: better estimation of sto_frac in the beginning of the upchirps)
                    float tmp_sto_frac = estimate_STO_frac(); // better estimation of sto_frac in the beginning of the upchirps
                    // Assign float diff_sto_frac to m_sto_frac - tmp_sto_frac.
                    float diff_sto_frac = m_sto_frac - tmp_sto_frac;

                    // Branch when condition (abs(diff_sto_frac) <= float(m_os_factor - 1) / m_os_factor) evaluates to true. (inline comment notes: avoid introducing off-by-one errors by estimating fine_sto=-0.499 , rough_sto=0.499)
                    if (abs(diff_sto_frac) <= float(m_os_factor - 1) / m_os_factor) // avoid introducing off-by-one errors by estimating fine_sto=-0.499 , rough_sto=0.499
                        // Assign m_sto_frac to tmp_sto_frac.
                        m_sto_frac = tmp_sto_frac;

                    // get SNR estimate from preamble
                    // downsample preab_raw
                    // Declare std::vector<gr_complex> corr_preamb.
                    std::vector<gr_complex> corr_preamb;
                    // Resize corr_preamb to (m_n_up_req + additional_upchirps) * m_number_of_bins, 0.
                    corr_preamb.resize((m_n_up_req + additional_upchirps) * m_number_of_bins, 0);
                    // apply sto correction
                    // Iterate with loop parameters (uint32_t i = 0; i < (m_n_up_req + additional_upchirps) * m_number_of_bins; i++).
                    for (uint32_t i = 0; i < (m_n_up_req + additional_upchirps) * m_number_of_bins; i++)
                    // Open a new scope block.
                    {
                        // Assign corr_preamb[i] to preamble_raw_up[m_os_factor * (m_number_of_bins - k_hat + i) - int(my_roundf(m_os_factor * m_sto_frac))].
                        corr_preamb[i] = preamble_raw_up[m_os_factor * (m_number_of_bins - k_hat + i) - int(my_roundf(m_os_factor * m_sto_frac))];
                    // Close the current scope block.
                    }
                    // Call std::rotate with arguments (corr_preamb.begin(), corr_preamb.begin() + mod(m_cfo_int, m_number_of_bins), corr_preamb.end()).
                    std::rotate(corr_preamb.begin(), corr_preamb.begin() + mod(m_cfo_int, m_number_of_bins), corr_preamb.end());
                    // apply cfo correction
                    // Call volk_32fc_x2_multiply_32fc with arguments (&corr_preamb[0], &corr_preamb[0], &CFO_int_correc[0], (m_n_up_req + additional_upchirps) * m_number_of_bins).
                    volk_32fc_x2_multiply_32fc(&corr_preamb[0], &corr_preamb[0], &CFO_int_correc[0], (m_n_up_req + additional_upchirps) * m_number_of_bins);
                    // Iterate with loop parameters (int i = 0; i < (m_n_up_req + additional_upchirps); i++).
                    for (int i = 0; i < (m_n_up_req + additional_upchirps); i++)
                    // Open a new scope block.
                    {
                        // Call volk_32fc_x2_multiply_32fc with arguments (&corr_preamb[m_number_of_bins * i], &corr_preamb[m_number_of_bins * i], &CFO_frac_correc[0], m_number_of_bins).
                        volk_32fc_x2_multiply_32fc(&corr_preamb[m_number_of_bins * i], &corr_preamb[m_number_of_bins * i], &CFO_frac_correc[0], m_number_of_bins);
                    // Close the current scope block.
                    }

                    // //apply sfo correction
                    // Call volk_32fc_x2_multiply_32fc with arguments (&corr_preamb[0], &corr_preamb[0], &sfo_corr_vect[0], (m_n_up_req + additional_upchirps) * m_number_of_bins).
                    volk_32fc_x2_multiply_32fc(&corr_preamb[0], &corr_preamb[0], &sfo_corr_vect[0], (m_n_up_req + additional_upchirps) * m_number_of_bins);

                    // Assign float snr_est to 0.
                    float snr_est = 0;
                    // Iterate with loop parameters (int i = 0; i < up_symb_to_use; i++).
                    for (int i = 0; i < up_symb_to_use; i++)
                    // Open a new scope block.
                    {
                        // Assign snr_est + to determine_snr(&corr_preamb[i * m_number_of_bins]).
                        snr_est += determine_snr(&corr_preamb[i * m_number_of_bins]);
                    // Close the current scope block.
                    }
                    // Assign snr_est / to up_symb_to_use.
                    snr_est /= up_symb_to_use;

                    // update sto_frac to its value at the beginning of the net id
                    // Assign m_sto_frac + to sfo_hat * m_preamb_len.
                    m_sto_frac += sfo_hat * m_preamb_len;
                    // ensure that m_sto_frac is in [-0.5,0.5]
                    // Branch when condition (abs(m_sto_frac) > 0.5) evaluates to true.
                    if (abs(m_sto_frac) > 0.5)
                    // Open a new scope block.
                    {
                        // Assign m_sto_frac to m_sto_frac + (m_sto_frac > 0 ? -1 : 1).
                        m_sto_frac = m_sto_frac + (m_sto_frac > 0 ? -1 : 1);
                    // Close the current scope block.
                    }
                    // decim net id according to new sto_frac and sto int
                    // Declare std::vector<gr_complex> net_ids_samp_dec.
                    std::vector<gr_complex> net_ids_samp_dec;
                    // Resize net_ids_samp_dec to 2 * m_number_of_bins, 0.
                    net_ids_samp_dec.resize(2 * m_number_of_bins, 0);
                    // start_off gives the offset in the net_id_samp vector required to be aligned in time (CFOint is equivalent to STOint since upchirp_val was forced to 0)
                    // Assign int start_off to (int)m_os_factor / 2 - (my_roundf(m_sto_frac * m_os_factor)) + m_os_factor * (.25 * m_number_of_bins + m_cfo_int).
                    int start_off = (int)m_os_factor / 2 - (my_roundf(m_sto_frac * m_os_factor)) + m_os_factor * (.25 * m_number_of_bins + m_cfo_int);
                    // Iterate with loop parameters (uint32_t i = 0; i < m_number_of_bins * 2; i++).
                    for (uint32_t i = 0; i < m_number_of_bins * 2; i++)
                    // Open a new scope block.
                    {
                        // Assign net_ids_samp_dec[i] to net_id_samp[start_off + i * m_os_factor].
                        net_ids_samp_dec[i] = net_id_samp[start_off + i * m_os_factor];
                    // Close the current scope block.
                    }
                    // Call volk_32fc_x2_multiply_32fc with arguments (&net_ids_samp_dec[0], &net_ids_samp_dec[0], &CFO_int_correc[0], 2 * m_number_of_bins).
                    volk_32fc_x2_multiply_32fc(&net_ids_samp_dec[0], &net_ids_samp_dec[0], &CFO_int_correc[0], 2 * m_number_of_bins);

                    // correct CFO_frac in the network ids
                    // Call volk_32fc_x2_multiply_32fc with arguments (&net_ids_samp_dec[0], &net_ids_samp_dec[0], &CFO_frac_correc[0], m_number_of_bins).
                    volk_32fc_x2_multiply_32fc(&net_ids_samp_dec[0], &net_ids_samp_dec[0], &CFO_frac_correc[0], m_number_of_bins);
                    // Call volk_32fc_x2_multiply_32fc with arguments (&net_ids_samp_dec[m_number_of_bins], &net_ids_samp_dec[m_number_of_bins], &CFO_frac_correc[0], m_number_of_bins).
                    volk_32fc_x2_multiply_32fc(&net_ids_samp_dec[m_number_of_bins], &net_ids_samp_dec[m_number_of_bins], &CFO_frac_correc[0], m_number_of_bins);

                    // Assign int netid1 to get_symbol_val(&net_ids_samp_dec[0], &m_downchirp[0]).
                    int netid1 = get_symbol_val(&net_ids_samp_dec[0], &m_downchirp[0]);
                    // Assign int netid2 to get_symbol_val(&net_ids_samp_dec[m_number_of_bins], &m_downchirp[0]).
                    int netid2 = get_symbol_val(&net_ids_samp_dec[m_number_of_bins], &m_downchirp[0]);
                    // Assign one_symbol_off to 0.
                    one_symbol_off = 0;

                    // Branch when condition (m_sync_words[0] == 0) evaluates to true. (inline comment notes: match netid1 only if requested)
                    if (m_sync_words[0] == 0) { // match netid1 only if requested
                        // Assign items_to_consume to 0.
                        items_to_consume = 0;
                        // Assign m_state to SFO_COMPENSATION.
                        m_state = SFO_COMPENSATION;
                        // Increment frame_cnt.
                        frame_cnt++;
                        // Invoke std::cout << "netid1 is " << netid1 << ", netid2 is " << netid2 <<.
                        std::cout << "netid1 is " << netid1 << ", netid2 is " << netid2 <<
                            // Declare ", check skipped" << std::endl.
                            ", check skipped" << std::endl;
                    // Close the current scope block.
                    }
                    // Define the f function. (inline comment notes: wrong id 1, (we allow an offset of 2))
                    else if (abs(netid1 - (int32_t)m_sync_words[0]) > 2) // wrong id 1, (we allow an offset of 2)
                    // Open a new scope block.
                    {

                        // check if we are in fact checking the second net ID and that the first one was considered as a preamble upchirp
                        // Branch when condition (abs(netid1 - (int32_t)m_sync_words[1]) <= 2) evaluates to true.
                        if (abs(netid1 - (int32_t)m_sync_words[1]) <= 2)
                        // Open a new scope block.
                        {
                            // Assign net_id_off to netid1 - (int32_t)m_sync_words[1].
                            net_id_off = netid1 - (int32_t)m_sync_words[1];
                            // Iterate with loop parameters (int i = m_preamb_len - 2; i < (m_n_up_req + additional_upchirps); i++).
                            for (int i = m_preamb_len - 2; i < (m_n_up_req + additional_upchirps); i++)
                            // Open a new scope block.
                            {
                                // Branch when condition (get_symbol_val(&corr_preamb[i * m_number_of_bins], &m_downchirp[0]) + net_id_off == m_sync_words[0]) evaluates to true. (inline comment notes: found the first netID)
                                if (get_symbol_val(&corr_preamb[i * m_number_of_bins], &m_downchirp[0]) + net_id_off == m_sync_words[0]) // found the first netID
                                // Open a new scope block.
                                {
                                    // Assign one_symbol_off to 1.
                                    one_symbol_off = 1;
                                    // Branch when condition (net_id_off != 0 && abs(net_id_off) > 1) evaluates to true.
                                    if (net_id_off != 0 && abs(net_id_off) > 1)
                                        // Declare std::cout << RED << "[frame_sync_impl.cc] net id offset >1: " << net_id_off << RESET << std::endl.
                                        std::cout << RED << "[frame_sync_impl.cc] net id offset >1: " << net_id_off << RESET << std::endl;
                                    // Branch when condition (m_should_log) evaluates to true.
                                    if (m_should_log)
                                        // Assign off_by_one_id to net_id_off != 0.
                                        off_by_one_id = net_id_off != 0;
                                    // Assign items_to_consume to -m_os_factor * net_id_off.
                                    items_to_consume = -m_os_factor * net_id_off;
                                    // the first symbol was mistaken for the end of the downchirp. we should correct and output it.

                                    // Assign int start_off to (int)m_os_factor / 2 - my_roundf(m_sto_frac * m_os_factor) + m_os_factor * (0.25 * m_number_of_bins + m_cfo_int).
                                    int start_off = (int)m_os_factor / 2 - my_roundf(m_sto_frac * m_os_factor) + m_os_factor * (0.25 * m_number_of_bins + m_cfo_int);
                                    // Iterate with loop parameters (int i = start_off; i < 1.25 * m_samples_per_symbol; i += m_os_factor).
                                    for (int i = start_off; i < 1.25 * m_samples_per_symbol; i += m_os_factor)
                                    // Open a new scope block.
                                    {

                                        // Assign out[int((i - start_off) / m_os_factor)] to additional_symbol_samp[i].
                                        out[int((i - start_off) / m_os_factor)] = additional_symbol_samp[i];
                                    // Close the current scope block.
                                    }
                                    // Assign items_to_output to m_number_of_bins.
                                    items_to_output = m_number_of_bins;
                                    // Assign m_state to SFO_COMPENSATION.
                                    m_state = SFO_COMPENSATION;
                                    // Assign symbol_cnt to 1.
                                    symbol_cnt = 1;
                                    // Increment frame_cnt.
                                    frame_cnt++;
                                // Close the current scope block.
                                }
                            // Close the current scope block.
                            }
                            // Branch when condition (!one_symbol_off) evaluates to true.
                            if (!one_symbol_off)
                            // Open a new scope block.
                            {
                                // Assign m_state to DETECT.
                                m_state = DETECT;
                                // Assign symbol_cnt to 1.
                                symbol_cnt = 1;
                                // Assign items_to_output to 0.
                                items_to_output = 0;
                                // Assign k_hat to 0.
                                k_hat = 0;
                                // Assign m_sto_frac to 0.
                                m_sto_frac = 0;
                                // Assign items_to_consume to 0.
                                items_to_consume = 0;
                            // Close the current scope block.
                            }
                        // Close the current scope block.
                        }
                        // Handle the alternative branch when previous conditions fail.
                        else
                        // Open a new scope block.
                        {
                            // Assign m_state to DETECT.
                            m_state = DETECT;
                            // Assign symbol_cnt to 1.
                            symbol_cnt = 1;
                            // Assign items_to_output to 0.
                            items_to_output = 0;
                            // Assign k_hat to 0.
                            k_hat = 0;
                            // Assign m_sto_frac to 0.
                            m_sto_frac = 0;
                            // Assign items_to_consume to 0.
                            items_to_consume = 0;
                        // Close the current scope block.
                        }
                    // Close the current scope block.
                    }
                    // Handle the alternative branch when previous conditions fail. (inline comment notes: net ID 1 valid)
                    else // net ID 1 valid
                    // Open a new scope block.
                    {
                        // Assign net_id_off to netid1 - (int32_t)m_sync_words[0].
                        net_id_off = netid1 - (int32_t)m_sync_words[0];
                        // Branch when condition (m_sync_words[1] != 0 &) evaluates to true. (inline comment notes: match netid2 only if requested)
                        if (m_sync_words[1] != 0 && // match netid2 only if requested
                            // Define the d function. (inline comment notes: wrong id 2)
                            mod(netid2 - net_id_off, m_number_of_bins) != (int32_t)m_sync_words[1]) // wrong id 2
                        // Open a new scope block.
                        {
                            // Assign m_state to DETECT.
                            m_state = DETECT;
                            // Assign symbol_cnt to 1.
                            symbol_cnt = 1;
                            // Assign items_to_output to 0.
                            items_to_output = 0;
                            // Assign k_hat to 0.
                            k_hat = 0;
                            // Assign m_sto_frac to 0.
                            m_sto_frac = 0;
                            // Assign items_to_consume to 0.
                            items_to_consume = 0;
                        // Close the current scope block.
                        }
                        // Handle the alternative branch when previous conditions fail.
                        else
                        // Open a new scope block.
                        {
                            // Branch when condition (net_id_off != 0 && abs(net_id_off) > 1) evaluates to true.
                            if (net_id_off != 0 && abs(net_id_off) > 1)
                                // Declare std::cout << RED << "[frame_sync_impl.cc] net id offset >1: " << net_id_off << RESET << std::endl.
                                std::cout << RED << "[frame_sync_impl.cc] net id offset >1: " << net_id_off << RESET << std::endl;
                            // Branch when condition (m_should_log) evaluates to true.
                            if (m_should_log)
                                // Assign off_by_one_id to net_id_off != 0.
                                off_by_one_id = net_id_off != 0;
                            // Assign items_to_consume to -m_os_factor * net_id_off.
                            items_to_consume = -m_os_factor * net_id_off;
                            // Assign m_state to SFO_COMPENSATION.
                            m_state = SFO_COMPENSATION;
                            // Increment frame_cnt.
                            frame_cnt++;
                            // Branch when condition (m_sync_words[1] == 0) evaluates to true.
                            if (m_sync_words[1] == 0)
                                // Declare std::cout << "netid2 is " << netid2 << std::endl.
                                std::cout << "netid2 is " << netid2 << std::endl;
                        // Close the current scope block.
                        }
                    // Close the current scope block.
                    }
                    // Branch when condition (m_state != DETECT) evaluates to true.
                    if (m_state != DETECT)
                    // Open a new scope block.
                    {
                        // update sto_frac to its value at the payload beginning
                        // Assign m_sto_frac + to sfo_hat * 4.25.
                        m_sto_frac += sfo_hat * 4.25;
                        // Assign sfo_cum to ((m_sto_frac * m_os_factor) - my_roundf(m_sto_frac * m_os_factor)) / m_os_factor.
                        sfo_cum = ((m_sto_frac * m_os_factor) - my_roundf(m_sto_frac * m_os_factor)) / m_os_factor;

                        // Assign pmt::pmt_t frame_info to pmt::make_dict().
                        pmt::pmt_t frame_info = pmt::make_dict();
                        // Assign frame_info to pmt::dict_add(frame_info, pmt::intern("is_header"), pmt::from_bool(true)).
                        frame_info = pmt::dict_add(frame_info, pmt::intern("is_header"), pmt::from_bool(true));
                        // Assign frame_info to pmt::dict_add(frame_info, pmt::intern("cfo_int"), pmt::mp((long)m_cfo_int)).
                        frame_info = pmt::dict_add(frame_info, pmt::intern("cfo_int"), pmt::mp((long)m_cfo_int));
                        // Assign frame_info to pmt::dict_add(frame_info, pmt::intern("cfo_frac"), pmt::mp((float)m_cfo_frac)).
                        frame_info = pmt::dict_add(frame_info, pmt::intern("cfo_frac"), pmt::mp((float)m_cfo_frac));
                        // Assign frame_info to pmt::dict_add(frame_info, pmt::intern("sf"), pmt::mp((long)m_sf)).
                        frame_info = pmt::dict_add(frame_info, pmt::intern("sf"), pmt::mp((long)m_sf));

                        // Call add_item_tag with arguments (0, nitems_written(0), pmt::string_to_symbol("frame_info"), frame_info).
                        add_item_tag(0, nitems_written(0), pmt::string_to_symbol("frame_info"), frame_info);

                        // Assign m_received_head to false.
                        m_received_head = false;
                        // Assign items_to_consume + to m_samples_per_symbol / 4 + m_os_factor * m_cfo_int.
                        items_to_consume += m_samples_per_symbol / 4 + m_os_factor * m_cfo_int;
                        // Assign symbol_cnt to one_symbol_off.
                        symbol_cnt = one_symbol_off;
                        // Assign float snr_est2 to 0.
                        float snr_est2 = 0;

                        // Branch when condition (m_should_log) evaluates to true.
                        if (m_should_log)
                        // Open a new scope block.
                        {
                            // estimate SNR

                            // Iterate with loop parameters (int i = 0; i < up_symb_to_use; i++).
                            for (int i = 0; i < up_symb_to_use; i++)
                            // Open a new scope block.
                            {
                                // Assign snr_est2 + to determine_snr(&preamble_upchirps[i * m_number_of_bins]).
                                snr_est2 += determine_snr(&preamble_upchirps[i * m_number_of_bins]);
                            // Close the current scope block.
                            }
                            // Assign snr_est2 / to up_symb_to_use.
                            snr_est2 /= up_symb_to_use;
                            // Assign float cfo_log to m_cfo_int + m_cfo_frac.
                            float cfo_log = m_cfo_int + m_cfo_frac;
                            // Assign float sto_log to k_hat - m_cfo_int + m_sto_frac.
                            float sto_log = k_hat - m_cfo_int + m_sto_frac;
                            // Assign float srn_log to snr_est.
                            float srn_log = snr_est;
                            // Assign float sfo_log to sfo_hat.
                            float sfo_log = sfo_hat;

                            // Assign sync_log_out[0] to srn_log.
                            sync_log_out[0] = srn_log;
                            // Assign sync_log_out[1] to cfo_log.
                            sync_log_out[1] = cfo_log;
                            // Assign sync_log_out[2] to sto_log.
                            sync_log_out[2] = sto_log;
                            // Assign sync_log_out[3] to sfo_log.
                            sync_log_out[3] = sfo_log;
                            // Assign sync_log_out[4] to off_by_one_id.
                            sync_log_out[4] = off_by_one_id;
                            // Call produce with arguments (1, 5).
                            produce(1, 5);
                        // Close the current scope block.
                        }
// Only compile the following section when PRINT_INFO is defined.
#ifdef PRINT_INFO

                        // Declare std::cout << "[frame_sync_impl.cc] " << frame_cnt << " CFO estimate: " << m_cfo_int + m_cfo_frac << ", STO estimate: " << k_hat - m_cfo_int + m_sto_frac << " snr est: " << snr_est << std::endl.
                        std::cout << "[frame_sync_impl.cc] " << frame_cnt << " CFO estimate: " << m_cfo_int + m_cfo_frac << ", STO estimate: " << k_hat - m_cfo_int + m_sto_frac << " snr est: " << snr_est << std::endl;
// Close the preceding conditional compilation block.
#endif
                    // Close the current scope block.
                    }
                // Close the current scope block.
                }
                // Close the current scope block.
                }

                // Exit the nearest enclosing loop or switch.
                break;
            // Close the current scope block.
            }
            // Handle switch label case SFO_COMPENSATION.
            case SFO_COMPENSATION:
            // Open a new scope block.
            {
                // transmit only useful symbols (at least 8 symbol for PHY header)

                // Branch when condition (symbol_cnt < 8 || ((uint32_t)symbol_cnt < m_symb_numb && m_received_head)) evaluates to true.
                if (symbol_cnt < 8 || ((uint32_t)symbol_cnt < m_symb_numb && m_received_head))
                // Open a new scope block.
                {
                    // output downsampled signal (with no STO but with CFO)
                    // Call memcpy with arguments (&out[0], &in_down[0], m_number_of_bins * sizeof(gr_complex)).
                    memcpy(&out[0], &in_down[0], m_number_of_bins * sizeof(gr_complex));
                    // Assign items_to_consume to m_samples_per_symbol.
                    items_to_consume = m_samples_per_symbol;

                    //   update sfo evolution
                    // Branch when condition (abs(sfo_cum) > 1.0 / 2 / m_os_factor) evaluates to true.
                    if (abs(sfo_cum) > 1.0 / 2 / m_os_factor)
                    // Open a new scope block.
                    {
                        // Assign items_to_consume - to (-2 * signbit(sfo_cum) + 1).
                        items_to_consume -= (-2 * signbit(sfo_cum) + 1);
                        // Assign sfo_cum - to (-2 * signbit(sfo_cum) + 1) * 1.0 / m_os_factor.
                        sfo_cum -= (-2 * signbit(sfo_cum) + 1) * 1.0 / m_os_factor;
                    // Close the current scope block.
                    }

                    // Assign sfo_cum + to sfo_hat.
                    sfo_cum += sfo_hat;

                    // Assign items_to_output to m_number_of_bins.
                    items_to_output = m_number_of_bins;
                    // Increment symbol_cnt.
                    symbol_cnt++;
                // Close the current scope block.
                }
                // Define the f function.
                else if (!m_received_head)
                // Open a new scope block. (inline comment notes: Wait for the header to be decoded)
                { // Wait for the header to be decoded
                    // Assign items_to_consume to 0.
                    items_to_consume = 0;
                    // Assign items_to_output to 0.
                    items_to_output = 0;
                // Close the current scope block.
                }
                // Handle the alternative branch when previous conditions fail.
                else
                // Open a new scope block.
                {
                    // Assign m_state to DETECT.
                    m_state = DETECT;
                    // Assign symbol_cnt to 1.
                    symbol_cnt = 1;
                    // Assign items_to_consume to m_samples_per_symbol.
                    items_to_consume = m_samples_per_symbol;
                    // Assign items_to_output to 0.
                    items_to_output = 0;
                    // Assign k_hat to 0.
                    k_hat = 0;
                    // Assign m_sto_frac to 0.
                    m_sto_frac = 0;
                // Close the current scope block.
                }
                // Exit the nearest enclosing loop or switch.
                break;
            // Close the current scope block.
            }
            // Handle default switch label.
            default:
            // Open a new scope block.
            {
                // Declare std::cerr << "[LoRa sync] WARNING : No state! Shouldn't happen\n".
                std::cerr << "[LoRa sync] WARNING : No state! Shouldn't happen\n";
                // Exit the nearest enclosing loop or switch.
                break;
            // Close the current scope block.
            }
            // Close the current scope block.
            }
            // Call consume_each with arguments (items_to_consume).
            consume_each(items_to_consume);
            // Call produce with arguments (0, items_to_output).
            produce(0, items_to_output);
            // Return WORK_CALLED_PRODUCE to the caller.
            return WORK_CALLED_PRODUCE;
        // Close the current scope block.
        }
    // Close the current scope and emit the trailing comment.
    } /* namespace lora_sdr */
// Close the current scope and emit the trailing comment.
} /* namespace gr */
