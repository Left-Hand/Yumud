#pragma once

#include "algebra/vectors/complex.hpp"
#include "algebra/vectors/polar.hpp"

using namespace ymd;

//IIR希尔伯特变换

template<typename T, size_t ORDER, size_t CHANNELS = 2>
class HilbertTransform {
private:
    std::array<std::array<T, ORDER>, CHANNELS> x;
    std::array<std::array<T, ORDER>, CHANNELS> y;
    size_t fs;

    static constexpr std::array<std::array<T, 2>, 1> BIQUAD_ORDER_2 = {{
        {-1.9799999f, 0.9801f}
    }};

    static constexpr std::array<std::array<T, 2>, 2> BIQUAD_ORDER_4 = {{
        {-1.9799999f, 0.9801f},
        {0.99999995f, 0.25f}
    }};

    static constexpr std::array<std::array<T, 2>, 3> BIQUAD_ORDER_6 = {{
        {-1.9799999f, 0.9801f},
        {-0.00000000f, 0.7225f},
        {0.99999995f, 0.25f}
    }};

    static constexpr std::array<std::array<T, 2>, 4> BIQUAD_ORDER_8 = {{
        {-1.9799999f, 0.9801f},
        {-0.4999093f, 0.25f},
        {0.4999093f, 0.25f},
        {0.99999995f, 0.25f}
    }};

    static constexpr std::array<std::array<T, 2>, 5> BIQUAD_ORDER_10 = {{
        {-1.9799999f, 0.9801f},
        {-0.7069957f, 0.25f},
        {-0.00000000f, 0.7225f},
        {0.7069957f, 0.25f},
        {0.99999995f, 0.25f}
    }};

    static constexpr std::array<std::array<T, 2>, 6> BIQUAD_ORDER_12 = {{
        {-1.9799999f, 0.9801f},
        {-0.8089062f, 0.25f},
        {-0.30895724f, 0.25f},
        {0.30895724f, 0.25f},
        {0.8089062f, 0.25f},
        {0.99999995f, 0.25f}
    }};

    static constexpr std::array<std::array<T, 2>, 7> BIQUAD_ORDER_14 = {{
        {-1.9799999f, 0.9801f},
        {-0.86592067f, 0.25f},
        {-0.4999093f, 0.25f},
        {-0.00000000f, 0.7225f},
        {0.4999093f, 0.25f},
        {0.86592067f, 0.25f},
        {0.99999995f, 0.25f}
    }};

    static constexpr std::array<std::array<T, 2>, 8> BIQUAD_ORDER_16 = {{
        {-1.9799999f, 0.9801f},
        {-0.90087148f, 0.25f},
        {-0.62338453f, 0.25f},
        {-0.22247718f, 0.25f},
        {0.22247718f, 0.25f},
        {0.62338453f, 0.25f},
        {0.90087148f, 0.25f},
        {0.99999995f, 0.25f}
    }};
// private:
//     template<size_t N, typename Dummy = void>
//     struct [[nodiscard]] biquad_coefficients;

//     template<typename Dummy = void>
//     struct [[nodiscard]] biquad_coefficients<4>{static constexpr auto value = BIQUAD_ORDER_4;}

public:

    constexpr explicit HilbertTransform(size_t sample_rate_val) : 
        fs(sample_rate_val
    ) {
        static_assert(ORDER > 0 && CHANNELS > 0 && ORDER <= 16 && ORDER % 2 == 0, 
            "ORDER must be a positive even integer <= 16, CHANNELS must be positive");
        
        // Initialize arrays with zeros
        for (size_t ch = 0; ch < CHANNELS; ++ch) {
            for (size_t i = 0; i < ORDER; ++i) {
                x[ch][i] = 0;
                y[ch][i] = 0;
            }
        }
    }

    [[nodiscard]] static constexpr const auto & biquad_coefficients() {
        if constexpr (ORDER == 2) {
            return BIQUAD_ORDER_2;
        } else if constexpr (ORDER == 4) {
            return BIQUAD_ORDER_4;
        } else if constexpr (ORDER == 6) {
            return BIQUAD_ORDER_6;
        } else if constexpr (ORDER == 8) {
            return BIQUAD_ORDER_8;
        } else if constexpr (ORDER == 10) {
            return BIQUAD_ORDER_10;
        } else if constexpr (ORDER == 12) {
            return BIQUAD_ORDER_12;
        } else if constexpr (ORDER == 14) {
            return BIQUAD_ORDER_14;
        } else if constexpr (ORDER == 16) {
            return BIQUAD_ORDER_16;
        } else {
            static_assert(ORDER <= 16 && ORDER % 2 == 0, "Invalid order");
            __builtin_unreachable();
        }
    }

    /// Apply the Hilbert transform to the given samples.
    constexpr void apply_transform(std::span<const T, CHANNELS> & samples) {
        const auto & biquad_coefficients = biquad_coefficients();

        for (size_t ch = 0; ch < CHANNELS; ++ch) {
            T input = samples[ch];
            
            for (size_t section = 0; section < biquad_coefficients.size(); ++section) {
                const auto& coeffs = biquad_coefficients[section];
                T a1 = coeffs[0];
                T a2 = coeffs[1];
                
                size_t idx = section * 2;
                T x_hist1 = x[ch][idx];
                T x_hist2 = x[ch][idx + 1];
                T y_hist1 = y[ch][idx];
                T y_hist2 = y[ch][idx + 1];
                
                T output = a2 * input +
                            a1 * x_hist1 +
                            x_hist2 -
                            a1 * y_hist1 -
                            a2 * y_hist2;
                
                // Update history
                x[ch][idx + 1] = x_hist1;
                x[ch][idx] = input;
                y[ch][idx + 1] = y_hist1;
                y[ch][idx] = output;
                
                input = output;
            }
            
            samples[ch] = input;
        }
    }

    /// Calculate the complex response of the filter at the given frequency.
    /// 
    /// Returns a pair of the amplitude and phase in radians.
    constexpr math::Complex<T> complex_response(T freq) const {
        T frequency = static_cast<T>(TAU) * freq / static_cast<T>(fs);

        T cos_f = cos(frequency);
        T sin_f = sin(frequency);
        T cos_2f = 2.0f * cos_f * cos_f - 1.0f;
        T sin_2f = 2.0f * cos_f * sin_f;

        T amplitude_out = 1;
        T phase_out = 0;

        const auto& biquad_coefficients = biquad_coefficients();

        for (const auto& coeffs : biquad_coefficients) {
            T a1 = coeffs[0];
            T a2 = coeffs[1];

            T real_n = a2 + a1 * cos_f + cos_2f;
            T imag_n = -a1 * sin_f - sin_2f;

            T real_d = 1.0f + a1 * cos_f + a2 * cos_2f;
            T imag_d = -a1 * sin_f - a2 * sin_2f;

            T amplitude = hypot(real_n, imag_n) / hypot(real_d, imag_d);
            T phase = fmod(atan2(imag_n, real_n) - atan2(imag_d, real_d) + static_cast<T>(TAU), static_cast<T>(TAU));

            amplitude_out *= amplitude;
            phase_out += phase;
            phase_out = fmod(phase_out, static_cast<T>(TAU));
        }

        return {amplitude_out, phase_out};
    }

    // For compatibility with potential effect processing interfaces
    void process(std::array<T, CHANNELS>& samples) {
        apply_transform(samples);
    }

    size_t delay() const {
        return 0;
    }


};