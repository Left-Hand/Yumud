#pragma once

#include "core/math/iq/fixed_t.hpp"
#include "core/utils/Option.hpp"
#include "core/utils/serde.hpp"

namespace ymd::dsp{
enum class CalibrateError:uint8_t{
    SampleIndexOutOfRange,
    DuplicatedSectorSampleDetected,
    ExpectedAndMeasuredExceedLimit
};



struct CalibrateSample{
    constexpr CalibrateSample():
        expected_(0),
        measured_(0){;}
    constexpr uq32 expected() const {
        return (expected_);
    }

    constexpr uq32 measured() const {
        return (measured_);
    }


    constexpr iq31 to_inaccuracy() const {
        return math::errmod(iq20(expected() - measured()), 0.02_iq20);
    }
private:
    uq32 expected_;
    uq32 measured_;
};


//储存了压缩保存的期望数据和实际数据 用于校准点分析
struct PackedCalibrateSample{
public:
    static constexpr size_t IGNORE_BITS = 5;
    // static constexpr auto MAX_ERR = iq31(q30(1.0 / (1 << IGNORE_BITS)));
    static constexpr auto MAX_ERR = iq31(1.0 / 5);
    // static constexpr q24 MAX_ERR = q24(1.0 / (1 << IGNORE_BITS));

    constexpr PackedCalibrateSample():
        expected_packed_data_(0),
        measured_packed_data_(0){;}

    static constexpr Option<PackedCalibrateSample> 
    from_expected_and_measure(const Angular<uq32> expected, const Angular<uq32> measured){
        const auto expected_packed_data = ({
            const auto may = real_to_packed(expected);
            if(may.is_none()) return None;
            may.unwrap();
        });

        const auto measured_packed_data = ({
            const auto may = real_to_packed(measured);
            if(may.is_none()) return None;
            may.unwrap();
        });

        return Some(PackedCalibrateSample(
            expected_packed_data,
            measured_packed_data
        ));
    }

    constexpr iq31 expected() const {
        return packed_to_real(expected_packed_data_);
    }

    constexpr iq31 measured() const {
        return packed_to_real(measured_packed_data_);
    }

    constexpr iq31 to_inaccuracy() const {
        return math::errmod(iq20(expected() - measured()), 0.02_iq20);
    }

private:
    constexpr PackedCalibrateSample(
        const uint16_t expected_packed_data, 
        const uint16_t measured_packed_data
    ):
        expected_packed_data_(expected_packed_data),
        measured_packed_data_(measured_packed_data){;}

    uint16_t expected_packed_data_;
    uint16_t measured_packed_data_;

    static constexpr Option<uint16_t> real_to_packed(const Angular<uq32> unpacked){
        return Some(uint16_t(unpacked.to_turns().to_bits() >> 16));
    }

    static constexpr iq31 packed_to_real(const uint16_t packed){
        return iq16::from_bits(packed);
    }
};


struct CompressedInaccuracy { 
    using Raw = uint16_t;

    constexpr CompressedInaccuracy ():
        raw_(0){;}

    static constexpr Option<CompressedInaccuracy> from_inaccuracy(const iq16 inaccuracy){
        if(is_input_valid(inaccuracy)) return None;
        return Some(CompressedInaccuracy(compress(inaccuracy)));
    }

    constexpr iq16 to_real() const{
        return decompress(raw_);
    }

    static constexpr bool is_input_valid(const iq16 inaccuracy){
        return ABS(inaccuracy) < 1;
    }

    static constexpr Raw compress(const iq16 count){
        return Raw(count.to_bits());
    }

    static constexpr iq16 decompress(const Raw raw){
        return iq16::from_bits(raw);
    }

private:
    constexpr CompressedInaccuracy (Raw raw):
        raw_(raw){;}

    Raw raw_;
};

}



namespace std {
    template <>
    struct tuple_size<::ymd::dsp::PackedCalibrateSample> 
        : integral_constant<size_t, 2> {};

    template <size_t N>
    struct tuple_element<N, ::ymd::dsp::PackedCalibrateSample> {
        using type = iq31;
    };

    template <>
    struct tuple_size<::ymd::dsp::CalibrateSample> 
        : integral_constant<size_t, 2> {};

    template <size_t N>
    struct tuple_element<N, ::ymd::dsp::CalibrateSample> {
        using type = iq31;
    };
}

