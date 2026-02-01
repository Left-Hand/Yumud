#include "src/testbench/tb.h"

#include "core/clock/time.hpp"
#include "core/utils/nth.hpp"
#include "core/utils/stdrange.hpp"
#include "core/utils/data_iter.hpp"
#include "primitive/arithmetic/rescaler.hpp"
#include "core/string/owned/heapless_string.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/hw_singleton.hpp"
#include "hal/gpio/gpio.hpp"
#include "hal/bus/uart/hw_singleton.hpp"

#include "dsp/z_transformation.hpp"
#include "primitive/arithmetic/db.hpp"

namespace ymd{

template<typename T>
static constexpr T rem_euclid(T a, T num){
    return fposmod(a, num);
}

}


template<typename T>
static constexpr T cubic_interpolate(
    std::array<T, 4> values,
	T t
){
    const auto c0 = values[1];
    const auto c1 = static_cast<T>(0.5) * (values[2] - values[0]);
    const auto c2 = values[0] - static_cast<T>(2.5) * values[1] + 
        2 * values[2] - static_cast<T>(0.5) * values[3];
    const auto c3 = static_cast<T>(0.5) * (values[3] - values[0]) + 1.5 * (values[1] - values[2]);

	return c0 + t * (c1 + t * (c2 + t * c3));
}





/// A Saturator effect.
/// 
/// The function is defined as: $f(x) := \frac{x}{1 + \frac{|x|}{a}^p} \cdot (1 + \frac{1}{a}^p)$
template<typename T>
struct [[nodiscard]] Saturator{
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





// static constexpr auto coeff = Z_TransferCoefficients<float, 3, 2>{{1, 0.5f, 0}, {1, 0.5f}};
// static constexpr auto resp = coeff.complex_response(1000.0f, 1000.0f);
// static_assert(resp.amplitude == 0.5f);

using namespace ymd::dsp;
template<typename T, size_t CHANNELS = 2>
struct [[nodiscard]] DownSampler{
    uint32_t target_sample_rate;
    uint32_t fs;
    T gain;
    T history[CHANNELS];
    T output[CHANNELS];
    T phase;
    uint32_t empty_count;
};

template<typename T>
struct [[nodiscard]] FilterParaments{
    T fs;
    T f0;
    T q;

    [[nodiscard]] constexpr std::pair<T, T> k_and_norm() const {
        T k = tan(M_PI * f0 / fs);
        T norm = 1 / (1 + k / q + k * k);
        return {k, norm};
    }
};


template<typename T>
struct [[nodiscard]] Biquad{
private:
    static constexpr T Q1 = static_cast<T>(0.70710677);



public:
    [[nodiscard]] static constexpr Z_TransferCoefficients<T, 3, 2>
    butterworth_hp(const FilterParaments<T> & para) {
        auto [k, norm] = para.k_and_norm();

        T b0 = 1 * norm;
        T b1 = (-2) * norm;
        T b2 = 1 * norm;
        T a1 = 2 * (k * k - 1) * norm;
        T a2 = (1 - k / para.q + k * k) * norm;

        return {std::array<T, 3>{b0, b1, b2}, std::array<T, 2>{a1, a2}};
    }

    [[nodiscard]] static constexpr Z_TransferCoefficients<T, 3, 2>
    butterworth_lp(const FilterParaments<T> & para) {
        auto [k, norm] = para.k_and_norm();

        T b0 = k * k * norm;
        T b1 = 2 * k * k * norm;
        T b2 = k * k * norm;
        T a1 = 2 * (k * k - 1) * norm;
        T a2 = (1 - k / para.q + k * k) * norm;

        return {std::array<T, 3>{b0, b1, b2}, std::array<T, 2>{a1, a2}};
    }

    [[nodiscard]] static constexpr Z_TransferCoefficients<T, 3, 2>
    butterworth_bp(const FilterParaments<T> & para) {
        auto [k, norm] = para.k_and_norm();
        
        T b0 = k * norm;
        T b1 = static_cast<T>(0.0);
        T b2 = -k * norm;
        T a1 = 2 * (k * k - 1) * norm;
        T a2 = (1 - k / para.q + k * k) * norm;

        return {std::array<T, 3>{b0, b1, b2}, std::array<T, 2>{a1, a2}};
    }

    [[nodiscard]] static constexpr Z_TransferCoefficients<T, 3, 2>
    butterworth_bs(const FilterParaments<T> & para) {
        auto [k, norm] = para.k_and_norm();
        
        T b0 = (1 + k * k) * norm;
        T b1 = 2 * (k * k - 1) * norm;
        T b2 = (1 + k * k) * norm;
        T a1 = 2 * (k * k - 1) * norm;
        T a2 = (1 - k / para.q + k * k) * norm;

        return {std::array<T, 3>{b0, b1, b2}, std::array<T, 2>{a1, a2}};
    }
};

#if 0
static constexpr auto coeff = Biquad<float>::butterworth_bp(100, 1000, 2);
static_assert(coeff.den[0] == 1.0f);
static_assert(coeff.den[1] == 1.0f);
#endif

template<typename T>
struct [[nodiscard]] SecondOrderFilterConfig{
    uint32_t fs;
    T fc;


    [[nodiscard]] constexpr Z_TransferCoefficients<T, 3, 3> to_coeffs() const {
        auto & self = *this;
        static constexpr T q_butterworth = T(0.7071);

        T b0, b1, b2, a0, a1, a2;

        const T w0 = T(TAU) * self.fc / self.fs;
        const T sin_w0 = T(sin((w0)));
        const T cos_w0 = T(cos((w0)));
        const T alpha = sin_w0 / (2 * q_butterworth);


        b0 = (1 - cos_w0) / 2;
        b1 = b0 * 2;
        b2 = b0;
        a0 = 1 + alpha;
        a1 = -2 * cos_w0;
        a2 = 1 - alpha;

        // a0_ = b0 / a0;
        // a1_ = b1 / a0;
        // a2_ = b2 / a0;
        // a3_ = a1 / a0;
        // a4_ = a2 / a0;

        return Z_TransferCoefficients<T, 3, 3>{};
    }
};