#pragma once

#include <algorithm>
#include <array>
#include <tuple>


#include "core/math/real.hpp"
#include "core/utils/Result.hpp"
#include "config.hpp"
#include "dsp/motor_ctrl/calibrate_table.hpp"

namespace ymd{

struct LapPosition{
    real_t position;
};

struct RawLapPosition{
    real_t position;
};

struct Elecrad{
    real_t elecrad;
};




struct CalibrateTableStorage{
    struct CompressedInaccuracy { 
        using Raw = uint16_t;

        constexpr CompressedInaccuracy ():
            raw_(0){;}

        explicit constexpr CompressedInaccuracy (const Raw raw):
            raw_(raw){;}

        static constexpr Option<CompressedInaccuracy> from(const q16 inaccuracy){
            if(is_input_valid(inaccuracy)) return None;
            return Some(CompressedInaccuracy(compress(inaccuracy)));
        }

        constexpr q16 to_real() const{
            return decompress(raw_);
        }

        static constexpr bool is_input_valid(const q16 inaccuracy){
            return ABS(inaccuracy) < 1;
        }

        static constexpr uint16_t compress(const q16 count){
            return uint16_t(count.as_i32());
        }

        static constexpr q16 decompress(const Raw raw){
            return q16::from_i32(raw);
        }

    private:
        Raw raw_;
    };

    std::array<CompressedInaccuracy, MOTOR_POLE_PAIRS> buf = {};

    constexpr Result<void, void> load_from_buf(std::span<const uint8_t> pbuf){
        if(pbuf.size() != sizeof(buf)) return Err();

        std::memcpy(buf.data(), pbuf.data(), sizeof(buf));
        return Ok();
    }
};

}