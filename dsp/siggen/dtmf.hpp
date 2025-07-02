#pragma once

#include "core/math/real.hpp"
#include "core/math/realmath.hpp"

class DoubleToneMultiFrequencySiggen{
public:
    struct Config{
        std::array<uint16_t, 4> fl_map;
        std::array<uint16_t, 4> fh_map;
        uint32_t fs;
    };

    DoubleToneMultiFrequencySiggen(const Config & cfg){
        reconf(cfg);
        reset();
    }

    void reconf(const Config & cfg){
        fl_map_ = cfg.fl_map;
        fh_map_ = cfg.fh_map;

        delta_ = real_t(1) / cfg.fs;
    }

    void reset(){
        fl_index_ = 0;
        fh_index_ = 0;
    }

    void update(const real_t now_seconds){
        const auto fl_ = fl_map_[fl_index_];
        const auto fh_ = fh_map_[fh_index_];

        const auto rad = real_t(TAU) * frac(now_seconds);
        result_ = sin(fl_ * rad) + sin(fh_ * rad);
    }

    auto fl() const{
        return fl_map_[fl_index_];
    }

    auto fh() const{
        return fh_map_[fh_index_];
    }

    auto result() const{
        return result_;
    }

    auto operator ()() const{
        return result();
    }
private:
    uint8_t fl_index_ = 0;
    uint8_t fh_index_ = 0;

    std::array<uint16_t, 4> fl_map_;
    std::array<uint16_t, 4> fh_map_;

    real_t delta_;
    real_t result_;
};