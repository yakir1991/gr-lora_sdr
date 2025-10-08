/* -*- c++ -*- */
/*
 * Standalone LoRa SDR receive chain for experimental use.
 * This file is part of gr-lora_sdr and distributed under the terms
 * of the GNU General Public License as published by the Free Software
 * Foundation; either version 3 of the License, or (at your option) any
 * later version.
 */

// Bring in declarations from "gnuradio/lora_sdr/standalone_rx_chain.h".
#include "gnuradio/lora_sdr/standalone_rx_chain.h"

// Bring in declarations from <algorithm>.
#include <algorithm>
// Bring in declarations from <cmath>.
#include <cmath>
// Bring in declarations from <cstddef>.
#include <cstddef>
// Bring in declarations from <iterator>.
#include <iterator>
// Bring in declarations from <stdexcept>.
#include <stdexcept>
// Bring in declarations from <utility>.
#include <utility>

// Start namespace  scope.
namespace {
// Assign constexpr double k_pi to 3.141592653589793238462643383279502884.
constexpr double k_pi = 3.141592653589793238462643383279502884;
// Close the current scope block.
}

// Start namespace gr scope.
namespace gr {
// Start namespace lora_sdr scope.
namespace lora_sdr {
// Start namespace experimental scope.
namespace experimental {

// Assign vector_source::vector_source() to default.
vector_source::vector_source() = default;

// Define the e::vector_source function.
vector_source::vector_source(complex_vector samples)
    // Initialize base classes or members with d_samples(std::move(samples)).
    : d_samples(std::move(samples))
// Open a new scope block.
{
// Close the current scope block.
}

// Define the e::set_samples function.
void vector_source::set_samples(const complex_vector& samples)
// Open a new scope block.
{
    // Assign d_samples to samples.
    d_samples = samples;
// Close the current scope block.
}

// Execute void vector_source::set_samples(complex_vector&& samples) noexcept.
void vector_source::set_samples(complex_vector&& samples) noexcept
// Open a new scope block.
{
    // Assign d_samples to std::move(samples).
    d_samples = std::move(samples);
// Close the current scope block.
}

// Execute const complex_vector& vector_source::samples() const noexcept.
const complex_vector& vector_source::samples() const noexcept
// Open a new scope block.
{
    // Return d_samples to the caller.
    return d_samples;
// Close the current scope block.
}

// Execute bool vector_source::empty() const noexcept.
bool vector_source::empty() const noexcept
// Open a new scope block.
{
    // Return d_samples.empty() to the caller.
    return d_samples.empty();
// Close the current scope block.
}

// Define the k::window_block function.
window_block::window_block()
    // Initialize base classes or members with d_size(0), d_type(window_type::hann).
    : d_size(0), d_type(window_type::hann)
// Open a new scope block.
{
// Close the current scope block.
}

// Define the k::window_block function.
window_block::window_block(size_t size, window_type type)
    // Initialize base classes or members with d_size(0), d_type(type).
    : d_size(0), d_type(type)
// Open a new scope block.
{
    // Branch when condition (size > 0) evaluates to true.
    if (size > 0) {
        // Call set_size with arguments (size).
        set_size(size);
    // Close the current scope block.
    }
// Close the current scope block.
}

// Execute size_t window_block::size() const noexcept.
size_t window_block::size() const noexcept
// Open a new scope block.
{
    // Return d_size to the caller.
    return d_size;
// Close the current scope block.
}

// Execute window_block::window_type window_block::type() const noexcept.
window_block::window_type window_block::type() const noexcept
// Open a new scope block.
{
    // Return d_type to the caller.
    return d_type;
// Close the current scope block.
}

// Define the k::set_size function.
void window_block::set_size(size_t size)
// Open a new scope block.
{
    // Branch when condition (size == 0) evaluates to true.
    if (size == 0) {
        // Assign d_size to 0.
        d_size = 0;
        // Clear all contents from d_coeffs.
        d_coeffs.clear();
        // Return to the caller.
        return;
    // Close the current scope block.
    }

    // Assign d_size to size.
    d_size = size;
    // Assign size, 1.0F to d_coeffs.
    d_coeffs.assign(size, 1.0F);
    // Call compute_coefficients with arguments ().
    compute_coefficients();
// Close the current scope block.
}

// Define the k::set_type function.
void window_block::set_type(window_type type)
// Open a new scope block.
{
    // Branch when condition (d_type != type) evaluates to true.
    if (d_type != type) {
        // Assign d_type to type.
        d_type = type;
        // Branch when condition (d_size > 0) evaluates to true.
        if (d_size > 0) {
            // Call compute_coefficients with arguments ().
            compute_coefficients();
        // Close the current scope block.
        }
    // Close the current scope block.
    }
// Close the current scope block.
}

// Execute const std::vector<float>& window_block::coefficients() const noexcept.
const std::vector<float>& window_block::coefficients() const noexcept
// Open a new scope block.
{
    // Return d_coeffs to the caller.
    return d_coeffs;
// Close the current scope block.
}

// Define the k::compute_coefficients function.
void window_block::compute_coefficients()
// Open a new scope block.
{
    // Branch when condition (d_size == 0) evaluates to true.
    if (d_size == 0) {
        // Return to the caller.
        return;
    // Close the current scope block.
    }

    // Branch when condition (d_size == 1) evaluates to true.
    if (d_size == 1) {
        // Assign d_coeffs[0] to 1.0F.
        d_coeffs[0] = 1.0F;
        // Return to the caller.
        return;
    // Close the current scope block.
    }

    // Assign const double n_minus_one to static_cast<double>(d_size - 1U).
    const double n_minus_one = static_cast<double>(d_size - 1U);
    // Iterate with loop parameters (size_t n = 0; n < d_size; ++n).
    for (size_t n = 0; n < d_size; ++n) {
        // Assign const double ratio to static_cast<double>(n) / n_minus_one.
        const double ratio = static_cast<double>(n) / n_minus_one;
        // Start the body of a do-while loop.
        double value = 1.0;

        // Select behavior based on (d_type).
        switch (d_type) {
        // Handle switch label case window_type.
        case window_type::rectangular:
            // Assign value to 1.0.
            value = 1.0;
            // Exit the nearest enclosing loop or switch.
            break;
        // Handle switch label case window_type.
        case window_type::hann:
            // Assign value to 0.5 - 0.5 * std::cos(2.0 * k_pi * ratio).
            value = 0.5 - 0.5 * std::cos(2.0 * k_pi * ratio);
            // Exit the nearest enclosing loop or switch.
            break;
        // Handle switch label case window_type.
        case window_type::hamming:
            // Assign value to 0.54 - 0.46 * std::cos(2.0 * k_pi * ratio).
            value = 0.54 - 0.46 * std::cos(2.0 * k_pi * ratio);
            // Exit the nearest enclosing loop or switch.
            break;
        // Handle switch label case window_type.
        case window_type::blackmanharris:
            // Execute value = 0.35875 - 0.48829 * std::cos(2.0 * k_pi * ratio) +.
            value = 0.35875 - 0.48829 * std::cos(2.0 * k_pi * ratio) +
                    // Execute 0.14128 * std::cos(4.0 * k_pi * ratio) -.
                    0.14128 * std::cos(4.0 * k_pi * ratio) -
                    // Execute statement 0.01168 * std::cos(6.0 * k_pi * ratio).
                    0.01168 * std::cos(6.0 * k_pi * ratio);
            // Exit the nearest enclosing loop or switch.
            break;
        // Close the current scope block.
        }

        // Assign d_coeffs[n] to static_cast<float>(value).
        d_coeffs[n] = static_cast<float>(value);
    // Close the current scope block.
    }
// Close the current scope block.
}

// Define the k::process const method.
complex_vector window_block::process(const complex_vector& input) const
// Open a new scope block.
{
    // Branch when condition (d_size == 0) evaluates to true.
    if (d_size == 0) {
        // Execute statement throw std::runtime_error("window_block is not configured").
        throw std::runtime_error("window_block is not configured");
    // Close the current scope block.
    }

    // Branch when condition (input.size() != d_size) evaluates to true.
    if (input.size() != d_size) {
        // Execute statement throw std::invalid_argument("window_block::process size mismatch").
        throw std::invalid_argument("window_block::process size mismatch");
    // Close the current scope block.
    }

    // Execute statement complex_vector output(d_size).
    complex_vector output(d_size);
    // Iterate with loop parameters (size_t n = 0; n < d_size; ++n).
    for (size_t n = 0; n < d_size; ++n) {
        // Assign output[n] to input[n] * d_coeffs[n].
        output[n] = input[n] * d_coeffs[n];
    // Close the current scope block.
    }

    // Return output to the caller.
    return output;
// Close the current scope block.
}

// Define the k::liquid_fft_block function.
liquid_fft_block::liquid_fft_block()
    // Initialize base classes or members with d_fft_size(0), d_plan(nullptr).
    : d_fft_size(0), d_plan(nullptr)
// Open a new scope block.
{
// Close the current scope block.
}

// Define the k::liquid_fft_block function.
liquid_fft_block::liquid_fft_block(size_t fft_size)
    // Initialize base classes or members with liquid_fft_block().
    : liquid_fft_block()
// Open a new scope block.
{
    // Branch when condition (fft_size > 0) evaluates to true.
    if (fft_size > 0) {
        // Call configure with arguments (fft_size).
        configure(fft_size);
    // Close the current scope block.
    }
// Close the current scope block.
}

// Define the k::~liquid_fft_block function.
liquid_fft_block::~liquid_fft_block()
// Open a new scope block.
{
    // Call destroy_plan with arguments ().
    destroy_plan();
// Close the current scope block.
}

// Execute size_t liquid_fft_block::size() const noexcept.
size_t liquid_fft_block::size() const noexcept
// Open a new scope block.
{
    // Return d_fft_size to the caller.
    return d_fft_size;
// Close the current scope block.
}

// Execute bool liquid_fft_block::is_configured() const noexcept.
bool liquid_fft_block::is_configured() const noexcept
// Open a new scope block.
{
    // Return d_plan != nullptr to the caller.
    return d_plan != nullptr;
// Close the current scope block.
}

// Define the k::destroy_plan function.
void liquid_fft_block::destroy_plan()
// Open a new scope block.
{
    // Branch when condition (d_plan != nullptr) evaluates to true.
    if (d_plan != nullptr) {
        // Call fft_destroy_plan with arguments (d_plan).
        fft_destroy_plan(d_plan);
        // Assign d_plan to nullptr.
        d_plan = nullptr;
    // Close the current scope block.
    }

    // Assign d_fft_size to 0.
    d_fft_size = 0;
    // Clear all contents from d_time_domain.
    d_time_domain.clear();
    // Clear all contents from d_frequency_domain.
    d_frequency_domain.clear();
// Close the current scope block.
}

// Define the k::configure function.
void liquid_fft_block::configure(size_t fft_size)
// Open a new scope block.
{
    // Branch when condition (fft_size == 0) evaluates to true.
    if (fft_size == 0) {
        // Execute statement throw std::invalid_argument("FFT size must be greater than zero").
        throw std::invalid_argument("FFT size must be greater than zero");
    // Close the current scope block.
    }

    // Branch when condition (fft_size == d_fft_size && d_plan != nullptr) evaluates to true.
    if (fft_size == d_fft_size && d_plan != nullptr) {
        // Return to the caller.
        return;
    // Close the current scope block.
    }

    // Call destroy_plan with arguments ().
    destroy_plan();

    // Assign d_fft_size to fft_size.
    d_fft_size = fft_size;
    // Assign fft_size, liquid_float_complex(0.0F, 0.0F) to d_time_domain.
    d_time_domain.assign(fft_size, liquid_float_complex(0.0F, 0.0F));
    // Assign fft_size, liquid_float_complex(0.0F, 0.0F) to d_frequency_domain.
    d_frequency_domain.assign(fft_size, liquid_float_complex(0.0F, 0.0F));

    // Specify parameter or initializer d_plan = fft_create_plan(static_cast<unsigned int>(fft_size).
    d_plan = fft_create_plan(static_cast<unsigned int>(fft_size),
                             // Specify parameter or initializer d_time_domain.data().
                             d_time_domain.data(),
                             // Specify parameter or initializer d_frequency_domain.data().
                             d_frequency_domain.data(),
                             // Specify parameter or initializer LIQUID_FFT_FORWARD.
                             LIQUID_FFT_FORWARD,
                             // Execute statement 0).
                             0);

    // Branch when condition (d_plan == nullptr) evaluates to true.
    if (d_plan == nullptr) {
        // Execute statement throw std::runtime_error("Failed to create liquid FFT plan").
        throw std::runtime_error("Failed to create liquid FFT plan");
    // Close the current scope block.
    }
// Close the current scope block.
}

// Define the k::process function.
complex_vector liquid_fft_block::process(const complex_vector& input)
// Open a new scope block.
{
    // Branch when condition (input.empty()) evaluates to true.
    if (input.empty()) {
        // Return {} to the caller.
        return {};
    // Close the current scope block.
    }

    // Branch when condition (input.size() != d_fft_size || d_plan == nullptr) evaluates to true.
    if (input.size() != d_fft_size || d_plan == nullptr) {
        // Call configure with arguments (input.size()).
        configure(input.size());
    // Close the current scope block.
    }

    // Define the d::transform function.
    std::transform(input.begin(), input.end(), d_time_domain.begin(), [](const complex_type& sample) {
        // Return liquid_float_complex(sample.real(), sample.imag()) to the caller.
        return liquid_float_complex(sample.real(), sample.imag());
    // Close the current scope and emit the trailing comment.
    });

    // Call fft_execute with arguments (d_plan).
    fft_execute(d_plan);

    // Return complex_vector(d_frequency_domain.begin(), d_frequency_domain.end()) to the caller.
    return complex_vector(d_frequency_domain.begin(), d_frequency_domain.end());
// Close the current scope block.
}

// Define the k::process const method.
std::vector<float> magnitude_block::process(const complex_vector& input) const
// Open a new scope block.
{
    // Execute statement std::vector<float> magnitudes(input.size(), 0.0F).
    std::vector<float> magnitudes(input.size(), 0.0F);

    // Define the d::transform function.
    std::transform(input.begin(), input.end(), magnitudes.begin(), [](const complex_type& value) {
        // Return std::abs(value) to the caller.
        return std::abs(value);
    // Close the current scope and emit the trailing comment.
    });

    // Return magnitudes to the caller.
    return magnitudes;
// Close the current scope block.
}

// Define the k::peak_detector_block function.
peak_detector_block::peak_detector_block(float threshold, bool relative)
    // Initialize base classes or members with d_threshold(threshold), d_relative(relative).
    : d_threshold(threshold), d_relative(relative)
// Open a new scope block.
{
// Close the current scope block.
}

// Execute float peak_detector_block::threshold() const noexcept.
float peak_detector_block::threshold() const noexcept
// Open a new scope block.
{
    // Return d_threshold to the caller.
    return d_threshold;
// Close the current scope block.
}

// Execute bool peak_detector_block::relative() const noexcept.
bool peak_detector_block::relative() const noexcept
// Open a new scope block.
{
    // Return d_relative to the caller.
    return d_relative;
// Close the current scope block.
}

// Execute void peak_detector_block::set_threshold(float threshold) noexcept.
void peak_detector_block::set_threshold(float threshold) noexcept
// Open a new scope block.
{
    // Assign d_threshold to threshold.
    d_threshold = threshold;
// Close the current scope block.
}

// Execute void peak_detector_block::set_relative(bool relative) noexcept.
void peak_detector_block::set_relative(bool relative) noexcept
// Open a new scope block.
{
    // Assign d_relative to relative.
    d_relative = relative;
// Close the current scope block.
}

// Execute void peak_detector_block::configure(float threshold, bool relative) noexcept.
void peak_detector_block::configure(float threshold, bool relative) noexcept
// Open a new scope block.
{
    // Assign d_threshold to threshold.
    d_threshold = threshold;
    // Assign d_relative to relative.
    d_relative = relative;
// Close the current scope block.
}

// Invoke std::optional<peak_detector_block::peak_info>.
std::optional<peak_detector_block::peak_info>
// Define the k::process const method.
peak_detector_block::process(const std::vector<float>& magnitudes) const
// Open a new scope block.
{
    // Branch when condition (magnitudes.empty()) evaluates to true.
    if (magnitudes.empty()) {
        // Return std::nullopt to the caller.
        return std::nullopt;
    // Close the current scope block.
    }

    // Declare auto-deduced variable auto it.
    auto it = std::max_element(magnitudes.begin(), magnitudes.end());
    // Declare constant const float max_value.
    const float max_value = *it;

    // Declare peak_info info.
    peak_info info;
    // Assign info.index to static_cast<size_t>(std::distance(magnitudes.begin(), it)).
    info.index = static_cast<size_t>(std::distance(magnitudes.begin(), it));
    // Assign info.value to max_value.
    info.value = max_value;

    // Assign float threshold_value to d_threshold.
    float threshold_value = d_threshold;
    // Branch when condition (d_relative) evaluates to true.
    if (d_relative) {
        // Assign threshold_value * to max_value.
        threshold_value *= max_value;
    // Close the current scope block.
    }

    // Branch when condition (threshold_value > 0.0F && max_value < threshold_value) evaluates to true.
    if (threshold_value > 0.0F && max_value < threshold_value) {
        // Return std::nullopt to the caller.
        return std::nullopt;
    // Close the current scope block.
    }

    // Return info to the caller.
    return info;
// Close the current scope block.
}

// Define the n::standalone_rx_chain function.
standalone_rx_chain::standalone_rx_chain(const config& cfg)
    // Initialize base classes or members with d_cfg(cfg),.
    : d_cfg(cfg),
      // Specify parameter or initializer d_source().
      d_source(),
      // Specify parameter or initializer d_window(cfg.fft_size, cfg.window).
      d_window(cfg.fft_size, cfg.window),
      // Specify parameter or initializer d_fft(cfg.fft_size).
      d_fft(cfg.fft_size),
      // Specify parameter or initializer d_magnitude().
      d_magnitude(),
      // Define the r function.
      d_detector(cfg.peak_threshold, cfg.relative_threshold)
// Open a new scope block.
{
    // Branch when condition (cfg.fft_size == 0) evaluates to true.
    if (cfg.fft_size == 0) {
        // Execute statement throw std::invalid_argument("FFT size must be greater than zero").
        throw std::invalid_argument("FFT size must be greater than zero");
    // Close the current scope block.
    }
// Close the current scope block.
}

// Define the n::set_config function.
void standalone_rx_chain::set_config(const config& cfg)
// Open a new scope block.
{
    // Branch when condition (cfg.fft_size == 0) evaluates to true.
    if (cfg.fft_size == 0) {
        // Execute statement throw std::invalid_argument("FFT size must be greater than zero").
        throw std::invalid_argument("FFT size must be greater than zero");
    // Close the current scope block.
    }

    // Assign d_cfg to cfg.
    d_cfg = cfg;
    // Execute statement d_window.set_type(cfg.window).
    d_window.set_type(cfg.window);
    // Execute statement d_window.set_size(cfg.fft_size).
    d_window.set_size(cfg.fft_size);
    // Execute statement d_fft.configure(cfg.fft_size).
    d_fft.configure(cfg.fft_size);
    // Execute statement d_detector.configure(cfg.peak_threshold, cfg.relative_threshold).
    d_detector.configure(cfg.peak_threshold, cfg.relative_threshold);
// Close the current scope block.
}

// Execute const standalone_rx_chain::config& standalone_rx_chain::get_config() const noexcept.
const standalone_rx_chain::config& standalone_rx_chain::get_config() const noexcept
// Open a new scope block.
{
    // Return d_cfg to the caller.
    return d_cfg;
// Close the current scope block.
}

// Execute window_block& standalone_rx_chain::window() noexcept.
window_block& standalone_rx_chain::window() noexcept
// Open a new scope block.
{
    // Return d_window to the caller.
    return d_window;
// Close the current scope block.
}

// Execute const window_block& standalone_rx_chain::window() const noexcept.
const window_block& standalone_rx_chain::window() const noexcept
// Open a new scope block.
{
    // Return d_window to the caller.
    return d_window;
// Close the current scope block.
}

// Execute peak_detector_block& standalone_rx_chain::detector() noexcept.
peak_detector_block& standalone_rx_chain::detector() noexcept
// Open a new scope block.
{
    // Return d_detector to the caller.
    return d_detector;
// Close the current scope block.
}

// Execute const peak_detector_block& standalone_rx_chain::detector() const noexcept.
const peak_detector_block& standalone_rx_chain::detector() const noexcept
// Open a new scope block.
{
    // Return d_detector to the caller.
    return d_detector;
// Close the current scope block.
}

// Define the n::process function.
rx_result standalone_rx_chain::process(const complex_vector& input)
// Open a new scope block.
{
    // Branch when condition (d_cfg.fft_size == 0) evaluates to true.
    if (d_cfg.fft_size == 0) {
        // Execute statement throw std::runtime_error("standalone_rx_chain is not configured").
        throw std::runtime_error("standalone_rx_chain is not configured");
    // Close the current scope block.
    }

    // Declare complex_vector prepared.
    complex_vector prepared;
    // Execute statement prepared.reserve(d_cfg.fft_size).
    prepared.reserve(d_cfg.fft_size);

    // Branch when condition (input.size() < d_cfg.fft_size) evaluates to true.
    if (input.size() < d_cfg.fft_size) {
        // Assign prepared to input.
        prepared = input;
        // Resize prepared to d_cfg.fft_size, complex_type(0.0F, 0.0F).
        prepared.resize(d_cfg.fft_size, complex_type(0.0F, 0.0F));
    // Close the current scope and emit the trailing comment.
    } else if (input.size() > d_cfg.fft_size) {
        // Assign input.begin( to prepared.
        prepared.assign(input.begin(),
                        // Call std::next with arguments (input.begin(), static_cast<std::ptrdiff_t>(d_cfg.fft_size))).
                        std::next(input.begin(), static_cast<std::ptrdiff_t>(d_cfg.fft_size)));
    // Close the current scope and emit the trailing comment.
    } else {
        // Assign prepared to input.
        prepared = input;
    // Close the current scope block.
    }

    // Execute statement d_source.set_samples(prepared).
    d_source.set_samples(prepared);

    // Branch when condition (d_window.size() != prepared.size()) evaluates to true.
    if (d_window.size() != prepared.size()) {
        // Execute statement d_window.set_size(prepared.size()).
        d_window.set_size(prepared.size());
    // Close the current scope block.
    }

    // Branch when condition (d_window.type() != d_cfg.window) evaluates to true.
    if (d_window.type() != d_cfg.window) {
        // Execute statement d_window.set_type(d_cfg.window).
        d_window.set_type(d_cfg.window);
    // Close the current scope block.
    }

    // Declare auto-deduced variable auto windowed.
    auto windowed = d_window.process(d_source.samples());

    // Branch when condition (!d_fft.is_configured() || d_fft.size() != windowed.size()) evaluates to true.
    if (!d_fft.is_configured() || d_fft.size() != windowed.size()) {
        // Execute statement d_fft.configure(windowed.size()).
        d_fft.configure(windowed.size());
    // Close the current scope block.
    }

    // Declare auto-deduced variable auto spectrum.
    auto spectrum = d_fft.process(windowed);
    // Declare auto-deduced variable auto magnitude.
    auto magnitude = d_magnitude.process(spectrum);

    // Branch when condition (d_cfg.normalize_magnitude && !magnitude.empty()) evaluates to true.
    if (d_cfg.normalize_magnitude && !magnitude.empty()) {
        // Declare auto-deduced variable auto max_it.
        auto max_it = std::max_element(magnitude.begin(), magnitude.end());
        // Branch when condition (max_it != magnitude.end() && *max_it > 0.0F) evaluates to true.
        if (max_it != magnitude.end() && *max_it > 0.0F) {
            // Declare constant const float inv_max.
            const float inv_max = 1.0F / *max_it;
            // Iterate with loop parameters (auto& value : magnitude).
            for (auto& value : magnitude) {
                // Assign value * to inv_max.
                value *= inv_max;
            // Close the current scope block.
            }
        // Close the current scope block.
        }
    // Close the current scope block.
    }

    // Branch when condition (d_detector.threshold() != d_cfg.peak_threshold || d_detector.relative() != d_cfg.relative_threshold) evaluates to true.
    if (d_detector.threshold() != d_cfg.peak_threshold || d_detector.relative() != d_cfg.relative_threshold) {
        // Execute statement d_detector.configure(d_cfg.peak_threshold, d_cfg.relative_threshold).
        d_detector.configure(d_cfg.peak_threshold, d_cfg.relative_threshold);
    // Close the current scope block.
    }

    // Declare auto-deduced variable auto peak.
    auto peak = d_detector.process(magnitude);

    // Return rx_result{std::move(prepared), to the caller.
    return rx_result{std::move(prepared),
                     // Specify parameter or initializer std::move(windowed).
                     std::move(windowed),
                     // Specify parameter or initializer std::move(spectrum).
                     std::move(spectrum),
                     // Specify parameter or initializer std::move(magnitude).
                     std::move(magnitude),
                     // Execute statement std::move(peak)}.
                     std::move(peak)};
// Close the current scope block.
}

// Close the current scope block. (inline comment notes: namespace experimental)
} // namespace experimental
// Close the current scope block. (inline comment notes: namespace lora_sdr)
} // namespace lora_sdr
// Close the current scope block. (inline comment notes: namespace gr)
} // namespace gr
