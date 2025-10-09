#include "src/testbench/tb.h"

#include "core/clock/time.hpp"
#include "core/utils/nth.hpp"
#include "core/utils/stdrange.hpp"
#include "core/utils/data_iter.hpp"
#include "core/utils/rescaler.hpp"
#include "core/string/fixed_string.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/uart/uartsw.hpp"
#include "hal/gpio/gpio.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "types/vectors/complex.hpp"

namespace ymd{

template<typename T>
static constexpr T rem_euclid(T a, T numerator){
    return fposmod(a, numerator);
}


template<typename T>
struct dB{
    constexpr explicit dB(T value):
        value_(value){}

    constexpr T to_linear() const{
        return pow(10, value_/10);
    }

    [[nodiscard]] constexpr dB operator -() const {
        return dB(-value_);
    }

    [[nodiscard]] constexpr dB operator +() const {
        return dB(value_);
    }

    constexpr dB & operator +=(const dB & rhs) {
        return dB(value_ + rhs.value_);
    }

    constexpr dB & operator -=(const dB & rhs) {
        return dB(value_ + rhs.value_);
    }

    [[nodiscard]] constexpr dB operator +(const dB & rhs) const {
        const auto ret = *this;
        ret += rhs;
        return ret;
    }

    [[nodiscard]] constexpr dB operator -(const dB & rhs) const {
        const auto ret = *this;
        ret -= rhs;
        return ret;
    }

    [[nodiscard]] constexpr dB scale(const T rhs){
        return dB(value_ * rhs);
    }

    [[nodiscard]] constexpr T operator *(const T rhs){
        return to_linear() * rhs;
    }
private:
    T value_;
};
}


template<typename T>
static constexpr T cubic_interpolate(
    std::array<T, 4> values,
	T t
){
    const auto c0 = values[1];
    const auto c1 = static_cast<T>(0.5) * (values[2] - values[0]);
    const auto c2 = values[0] - static_cast<T>(2.5) * values[1] + 
        static_cast<T>(2.0) * values[2] - static_cast<T>(0.5) * values[3];
    const auto c3 = static_cast<T>(0.5) * (values[3] - values[0]) + 1.5 * (values[1] - values[2]);

	return c0 + t * (c1 + t * (c2 + t * c3));
}

namespace ymd::file::wav{

enum class AudioFormat:uint16_t {
    PCM = 1

};
struct FileDescriptor{
    char     chunk_id[4]; //内容为"RIFF"
    uint32_t chunk_size;  //存储文件的字节数（不包含ChunkID和ChunkSize这8个字节）
    char     format[4];  //内容为"WAVE“

    [[nodiscard]] constexpr bool is_valid() const{
        return chunk_id[0] == 'R' && chunk_id[1] == 'I' && chunk_id[2] == 'F' && chunk_id[3] == 'F' &&
            format[0] == 'W' && format[1] == 'A' && format[2] == 'V' && format[3] == 'E';
    }
};

struct FileFormatSubchunk{
   char     subchunk1_id[4]; //内容为"fmt"
   uint32_t subchunk1_size;  //存储该子块的字节数（不含前面的Subchunk1ID和Subchunk1Size这8个字节）
   AudioFormat audio_format;    //存储音频文件的编码格式，例如若为PCM则其存储值为1。
   uint16_t num_channels;    //声道数，单声道(Mono)值为1，双声道(Stereo)值为2，等等
   uint32_t fs;     //采样率，如8k，44.1k等
   uint32_t byte_rate;       //每秒存储的bit数，其值 = SampleRate * NumChannels * BitsPerSample / 8
   uint16_t block_align;     //块对齐大小，其值 = NumChannels * BitsPerSample / 8
   uint16_t bits_per_sample;  //每个采样点的bit数，一般为8,16,32等。

    [[nodiscard]] constexpr bool is_valid() const{
        return subchunk1_id[0] == 'f' && subchunk1_id[1] == 'm' && subchunk1_id[2] == 't';
    }
};

struct FileDataSubchunk{
    char     subchunk2_id[4]; //内容为“data”

    //接下来的正式的数据部分的字节数，其值 == NumSamples * NumChannels * BitsPerSample / 8
    uint32_t subchunk2_size;  

    [[nodiscard]] constexpr bool is_valid() const{
        return subchunk2_id[0] == 'd' && subchunk2_id[1] == 'a' && subchunk2_id[2] == 't' && subchunk2_id[3] == 'a';
    }
};
}


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
//     struct biquad_coefficients;

//     template<typename Dummy = void>
//     struct biquad_coefficients<4>{static constexpr auto value = BIQUAD_ORDER_4;}

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
    constexpr Complex<T> complex_response(T freq) const {
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

/// A Saturator effect.
/// 
/// The function is defined as: $f(x) := \frac{x}{1 + \frac{|x|}{a}^p} \cdot (1 + \frac{1}{a}^p)$
template<typename T>
struct Saturator{
    float a;
    float p;
    float gain;

    constexpr void process(std::span<T> dst, std::span<const T> src) const {
        auto & self = *this;
        ASSERT(src.size() == dst.size());
        const size_t len = src.size(); 
		const auto factor = 1 + 1 / powf(self.a, self.p);
		for (size_t i = 0; i < len; i++) {
			const auto norm = powf((ABS(src[i]) / self.a), self.p);
			dst[i] = (src[i] / (1.0 + norm)) * self.gain * factor;
		}
    }
};


template<typename T>
struct AmplitudeAndPhase{
    T amplitude;
    Angle<T> phase;
};



template<typename T>
struct ResponseCalculator;


template<typename T, size_t NUM, size_t DEN>
struct Z_TransferCoefficients{
    std::array<T, NUM> numerator;
    std::array<T, DEN> denominator;

    using Self = Z_TransferCoefficients<T, NUM, DEN>;
    
    constexpr Z_TransferCoefficients() = default;
    
    constexpr Z_TransferCoefficients(const std::array<T, NUM>& num, const std::array<T, DEN>& den)
        : numerator(num), denominator(den) {}
    
    [[nodiscard]] constexpr const T& num(size_t index) const {
        return numerator[index];
    }
    
    [[nodiscard]] constexpr const T& den(size_t index) const {
        return denominator[index];
    }
    
    [[nodiscard]] constexpr T& num(size_t index) {
        return numerator[index];
    }
    
    [[nodiscard]] constexpr T& den(size_t index) {
        return denominator[index];
    }
    
    [[nodiscard]] static constexpr size_t num_size() {
        return NUM;
    }
    
    [[nodiscard]] static constexpr size_t den_size() {
        return DEN;
    }

    [[nodiscard]] constexpr AmplitudeAndPhase<T> complex_response(
        const auto freq, const auto fs
    ) const {
        return ResponseCalculator<Self>::calc_complex_response(*this, freq, fs);
    }
};

template<typename T>
struct ResponseCalculator<Z_TransferCoefficients<T, 3, 2>>{
    
    static constexpr AmplitudeAndPhase<T> calc_complex_response(
        const Z_TransferCoefficients<T, 3, 2>& self, 
        const auto freq, const auto fs
    ){
        const auto frequency = static_cast<T>(2.0 * PI) * freq / fs;

		const T cos_f = cos(frequency);
		const T sin_f = sin(frequency);
		const T cos_2f = 2 * cos_f * cos_f - 1;
		const T sin_2f = 2 * cos_f * sin_f;

        const Complex<T> n = {
            self.numerator[0] + self.numerator[1] * cos_f + self.numerator[2] * cos_2f,
            - (self.numerator[1] * sin_f + self.numerator[2] * sin_2f)
        };
		
        const Complex<T> d = {
            1 + self.denominator[0] * cos_f + self.denominator[1] * cos_2f,
            - (self.denominator[0] * sin_f + self.denominator[1] * sin_2f)
        };

		const auto amplitude = n.length() / d.length();
		const auto phase = (n.to_angle() - d.to_angle()).normalized();

		return {amplitude, phase};
    }
};

// static constexpr auto coeff = Z_TransferCoefficients<float, 3, 2>{{1, 0.5f, 0}, {1, 0.5f}};
// static constexpr auto resp = coeff.complex_response(1000.0f, 1000.0f);
// static_assert(resp.amplitude == 0.5f);

template<typename T, size_t CHANNELS = 2>
struct DownSampler{
    uint32_t target_sample_rate;
    uint32_t fs;
    T gain;
    T history[CHANNELS];
    T output[CHANNELS];
    T phase;
    uint32_t empty_count;
};

template<typename T>
struct FilterParaments{
    T fs;
    T f0;
    T q;
};


template<typename T>
struct Biquad{
private:
    static constexpr T Q1 = static_cast<T>(0.70710677);

    [[nodiscard]] static constexpr std::pair<T, T> get_k_and_norm(T fs, T f0, T q) {
        T k = tan(PI * f0 / fs);
        T norm = static_cast<T>(1.0) / (static_cast<T>(1.0) + k / q + k * k);
        return {k, norm};
    }

public:
    [[nodiscard]] static constexpr Z_TransferCoefficients<T, 3, 2>
    butterworth_hp(T fs, T f0, T q) {
        auto [k, norm] = get_k_and_norm(fs, f0, q);

        T b0 = static_cast<T>(1.0) * norm;
        T b1 = static_cast<T>(-2.0) * norm;
        T b2 = static_cast<T>(1.0) * norm;
        T a1 = static_cast<T>(2.0) * (k * k - static_cast<T>(1.0)) * norm;
        T a2 = (static_cast<T>(1.0) - k / q + k * k) * norm;

        return {std::array<T, 3>{b0, b1, b2}, std::array<T, 2>{a1, a2}};
    }

    [[nodiscard]] static constexpr Z_TransferCoefficients<T, 3, 2>
    butterworth_lp(T fs, T f0, T q) {
        auto [k, norm] = get_k_and_norm(fs, f0, q);

        T b0 = k * k * norm;
        T b1 = static_cast<T>(2.0) * k * k * norm;
        T b2 = k * k * norm;
        T a1 = static_cast<T>(2.0) * (k * k - static_cast<T>(1.0)) * norm;
        T a2 = (static_cast<T>(1.0) - k / q + k * k) * norm;

        return {std::array<T, 3>{b0, b1, b2}, std::array<T, 2>{a1, a2}};
    }

    [[nodiscard]] static constexpr Z_TransferCoefficients<T, 3, 2>
    butterworth_bp(T fs, T f0, T q) {
        auto [k, norm] = get_k_and_norm(fs, f0, q);
        
        T b0 = k * norm;
        T b1 = static_cast<T>(0.0);
        T b2 = -k * norm;
        T a1 = static_cast<T>(2.0) * (k * k - static_cast<T>(1.0)) * norm;
        T a2 = (static_cast<T>(1.0) - k / q + k * k) * norm;

        return {std::array<T, 3>{b0, b1, b2}, std::array<T, 2>{a1, a2}};
    }

    [[nodiscard]] static constexpr Z_TransferCoefficients<T, 3, 2>
    butterworth_bs(T fs, T f0, T q) {
        auto [k, norm] = get_k_and_norm(fs, f0, q);
        
        T b0 = (static_cast<T>(1.0) + k * k) * norm;
        T b1 = static_cast<T>(2.0) * (k * k - static_cast<T>(1.0)) * norm;
        T b2 = (static_cast<T>(1.0) + k * k) * norm;
        T a1 = static_cast<T>(2.0) * (k * k - static_cast<T>(1.0)) * norm;
        T a2 = (static_cast<T>(1.0) - k / q + k * k) * norm;

        return {std::array<T, 3>{b0, b1, b2}, std::array<T, 2>{a1, a2}};
    }
};

#if 0
static constexpr auto coeff = Biquad<float>::butterworth_bp(100, 1000, 2);
static_assert(coeff.denominator[0] == 1.0f);
static_assert(coeff.denominator[1] == 1.0f);
#endif