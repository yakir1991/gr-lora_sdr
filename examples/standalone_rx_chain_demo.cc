// Include the header that defines the experimental standalone receive chain class.
#include <gnuradio/lora_sdr/standalone_rx_chain.h>

// Include the complex number utilities used to represent IQ samples.
#include <complex>
// Include I/O manipulators for formatting numeric output.
#include <iomanip>
// Include the standard stream library for console printing.
#include <iostream>
// Include the standard dynamic array container.
#include <vector>

// Entry point of the demonstration executable.
int main()
{
    // Import the experimental namespace so we can access the receive chain classes directly.
    using namespace gr::lora_sdr::experimental;

    // Create a configuration structure that holds all runtime parameters.
    standalone_rx_chain::config cfg;
    // Configure the FFT size used to analyse the incoming tone.
    cfg.fft_size = 256;
    // Select a Hann window to taper the input samples.
    cfg.window = window_block::window_type::hann;
    // Set the absolute magnitude threshold that defines a peak.
    cfg.peak_threshold = 0.2F;
    // Instruct the detector to interpret the threshold relative to the maximum magnitude.
    cfg.relative_threshold = true;
    // Ask the processing chain to normalise the magnitude spectrum before peak detection.
    cfg.normalize_magnitude = true;

    // Build the standalone processing chain with the chosen configuration.
    standalone_rx_chain chain(cfg);

    // Allocate a vector that will hold one FFT frame of complex samples.
    std::vector<std::complex<float>> samples(cfg.fft_size);
    // Store a single-precision value of pi for phase computation.
    constexpr float pi = 3.14159265358979323846f;
    // Select the bin index that will contain the synthetic tone.
    const float tone_bin = 42.0F;

    // Generate a synthetic complex sinusoid occupying the chosen FFT bin.
    for (size_t n = 0; n < samples.size(); ++n) {
        // Compute the instantaneous phase for the current sample.
        const float phase = 2.0F * pi * tone_bin * static_cast<float>(n) /
                            static_cast<float>(cfg.fft_size);
        // Write a unit-magnitude complex exponential into the sample vector.
        samples[n] = std::polar(1.0F, phase);
    }

    // Process the generated tone through the receive chain and capture the results.
    auto result = chain.process(samples);

    // Report the number of samples that have been processed.
    std::cout << "Processed " << result.input.size() << " samples" << std::endl;

    // Print the detected peak bin if the detector found one above the threshold.
    if (result.peak) {
        // Output the dominant FFT bin index followed by its magnitude, formatted nicely.
        std::cout << "Dominant bin: " << result.peak->index << "\n"
                  << "Magnitude:   " << std::fixed << std::setprecision(3)
                  << result.peak->value << std::endl;
    } else {
        // Inform the user that no spectral peak exceeded the configured threshold.
        std::cout << "No peak above threshold" << std::endl;
    }

    // Return zero to indicate successful execution.
    return 0;
}
