#pragma once

#include "core/math/iq/iq_t.hpp"
#include "core/utils/Option.hpp"

namespace ymd::dsp{

template<size_t N>
static __fast_inline constexpr iq_t<N> errmod(const iq_t<N> x, const iq_t<N> s){
    const auto s_by_2 = s >> 1;
    iq_t<N> value = fmod(x + s_by_2, s) - s_by_2;
    if (value < 0) {
        value += s;
    }
    return value;
}

struct CalibrateSample{
    constexpr CalibrateSample():
        expected_(0),
        measured_(0){;}
    constexpr q31 expected() const {
        return (expected_);
    }

    constexpr q31 measured() const {
        return (measured_);
    }

    template <size_t N>
    constexpr auto get() const {
        if constexpr (N == 0) return expected();
        else if constexpr (N == 1) return measured();
    }

    constexpr q31 to_inaccuracy() const {
        return errmod(q20(expected() - measured()), 0.02_q20);
    }
private:
    q31 expected_;
    q31 measured_;
};


//储存了压缩保存的期望数据和实际数据 用于校准点分析
struct PackedCalibrateSample{
public:
    static constexpr size_t IGNORE_BITS = 5;
    // static constexpr auto MAX_ERR = q31(q30(1.0 / (1 << IGNORE_BITS)));
    static constexpr auto MAX_ERR = q31(1.0 / 5);
    // static constexpr q24 MAX_ERR = q24(1.0 / (1 << IGNORE_BITS));

    constexpr PackedCalibrateSample():
        expected_packed_data_(0),
        measured_packed_data_(0){;}

    static constexpr Option<PackedCalibrateSample> 
    from_expected_and_measure(const q31 expected, const q31 measured){
        const auto expected_packed_data = ({
            const auto opt = real_to_packed(expected);
            if(opt.is_none()) return None;
            opt.unwrap();
        });

        const auto measured_packed_data = ({
            const auto opt = real_to_packed(measured);
            if(opt.is_none()) return None;
            opt.unwrap();
        });

        return Some(PackedCalibrateSample(
            expected_packed_data,
            measured_packed_data
        ));
    }

    constexpr q31 expected() const {
        return packed_to_real(expected_packed_data_);
    }

    constexpr q31 measured() const {
        return packed_to_real(measured_packed_data_);
    }

    constexpr q31 to_inaccuracy() const {
        return errmod(q20(expected() - measured()), 0.02_q20);
    }

    template <size_t N>
    constexpr auto get() const {
        if constexpr (N == 0) return expected();
        else if constexpr (N == 1) return measured();
    }
private:
    constexpr PackedCalibrateSample(
        const uint16_t expected_packed, 
        const uint16_t measured_packed
    ):
        expected_packed_data_(expected_packed),
        measured_packed_data_(measured_packed){;}

    uint16_t expected_packed_data_;
    uint16_t measured_packed_data_;

    static constexpr Option<uint16_t> real_to_packed(const q16 unpacked){
        return Some(uint16_t(unpacked.as_i32()));
    }

    static constexpr q31 packed_to_real(const uint16_t packed){
        return q16::from_i32(packed);
    }
};

}

namespace std {
    template <>
    struct tuple_size<::ymd::dsp::PackedCalibrateSample> 
        : integral_constant<size_t, 2> {};

    template <size_t N>
    struct tuple_element<N, ::ymd::dsp::PackedCalibrateSample> {
        using type = ymd::q31;
    };

    template <>
    struct tuple_size<::ymd::dsp::CalibrateSample> 
        : integral_constant<size_t, 2> {};

    template <size_t N>
    struct tuple_element<N, ::ymd::dsp::CalibrateSample> {
        using type = ymd::q31;
    };
}

