#pragma once

#include "core/math/real.hpp"
#include "core/math/realmath.hpp"

namespace ymd::dsp{
class DoubleToneMultiFrequencySiggen{
public:
    struct Config{
        std::array<uint16_t, 4> fl_map;
        std::array<uint16_t, 4> fh_map;
        uint32_t fs;
    };

    constexpr DoubleToneMultiFrequencySiggen(const Config & cfg){
        reconf(cfg);
        reset();
    }

    constexpr void reconf(const Config & cfg){
        fl_map_ = cfg.fl_map;
        fh_map_ = cfg.fh_map;

        delta_ = iq16(1) / cfg.fs;
    }

    constexpr void reset(){
        fl_index_ = 0;
        fh_index_ = 0;
    }

    constexpr void update(const iq16 now_seconds){
        const auto fl_ = fl_map_[fl_index_];
        const auto fh_ = fh_map_[fh_index_];

        const auto rad = iq16(TAU) * iq16(frac(now_seconds));
        result_ = sin(fl_ * rad) + sin(fh_ * rad);
    }

    constexpr auto fl() const{
        return fl_map_[fl_index_];
    }

    constexpr auto fh() const{
        return fh_map_[fh_index_];
    }

    constexpr auto result() const{
        return result_;
    }

    constexpr auto operator ()() const{
        return result();
    }
private:
    uint8_t fl_index_ = 0;
    uint8_t fh_index_ = 0;

    std::array<uint16_t, 4> fl_map_;
    std::array<uint16_t, 4> fh_map_;

    iq16 delta_;
    iq16 result_;
};

}