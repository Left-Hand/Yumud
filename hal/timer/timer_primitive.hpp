#pragma once

#include "core/utils/sumtype.hpp"

namespace ymd{
class OutputStream;
}

namespace ymd::hal{

namespace details{
//pure function, easy test
static constexpr std::tuple<uint16_t, uint16_t> calc_best_arr_and_psc(
	const uint32_t periph_freq, 
	const uint32_t count_freq, 
	std::pair<uint16_t, uint16_t> arr_range
){
    const auto [min_arr, max_arr] = arr_range;
    const uint32_t target_div = periph_freq / count_freq;
    
    auto calc_psc_from_arr = [target_div](const uint16_t arr) -> uint16_t {
        return std::clamp<uint16_t>(int(target_div) / (int(arr) + 1) - 1, 0, 0xFFFF);
    };

    [[maybe_unused]]
    auto calc_arr_from_psc = [target_div](const uint16_t psc) -> uint16_t {
        return std::clamp<uint16_t>(int(target_div) / (int(psc) + 1) - 1, 0, 0xFFFF);
    };
    
    auto calc_freq_from_arr_and_psc = [periph_freq](const uint16_t arr, const uint16_t psc) -> uint32_t {
        return periph_freq / (arr + 1) / (psc + 1);
    };
    
    const auto min_psc = calc_psc_from_arr(max_arr);
    const auto max_psc = calc_psc_from_arr(min_arr);

    if (min_arr > max_arr) __builtin_abort();
    
    struct Best{
        uint16_t arr;
        uint16_t psc;
        uint32_t freq_err;
    };
    
    Best best{max_arr, min_psc, UINT32_MAX};
    for(int arr = max_arr; arr >= min_arr; arr--){
        const auto expect_psc = calc_psc_from_arr(arr);
        if((expect_psc >= max_psc) or (expect_psc < min_psc)) continue;
        
        std::optional<uint32_t> last_freq_;

        // const int psc_start = MAX(min_psc, expect_psc - 5);
        // const int psc_stop = MIN(max_psc, expect_psc + 5);
        // if(psc_start >= psc_stop) continue;

        // for(int psc = psc_start; psc < psc_stop; psc++){
        for(int psc = expect_psc - 2; psc < expect_psc + 2; psc++){
            const auto freq = calc_freq_from_arr_and_psc(arr, psc);
            if(last_freq_.has_value()){
                if((last_freq_.value() - count_freq) * (freq - count_freq) < 0) break;
            }else{
                last_freq_ = freq;
            }
            const auto freq_err = static_cast<uint32_t>(
                std::abs(int(freq) - int(count_freq))
            );
            if(freq_err < best.freq_err){
                if(freq_err == 0) return {uint16_t(arr), psc};
                best = {uint16_t(arr), uint16_t(psc), freq_err};
            }
        }
    }
    
    if(best.freq_err == UINT32_MAX) __builtin_abort();
    return {best.arr, best.psc};
}
}

struct [[nodiscard]] ArrAndPsc{
    using Self = ArrAndPsc;

    uint16_t arr;
    uint16_t psc;

    static constexpr ArrAndPsc from_nearest_count_freq(
        const uint32_t periph_freq,
        const uint32_t count_freq,
        std::pair<uint16_t, uint16_t> arr_range
    ){
        ArrAndPsc ret;
        std::tie(ret.arr, ret.psc) = details::calc_best_arr_and_psc(periph_freq, count_freq, arr_range);
        return ret;
    }

    friend OutputStream & operator <<(OutputStream & os, const Self & self);
};

struct [[nodiscard]] NearestFreq{
    using Self = NearestFreq;
    uint32_t count;

    friend OutputStream & operator <<(OutputStream & os, const Self & self);
};

struct [[nodiscard]] TimerCountFreq:
    public Sumtype<ArrAndPsc, NearestFreq>{
};




// enum class [[nodiscard]] TimerCountMode:uint8_t{
//     Up                      = 0x00,
//     Down                    = 0x01,
//     CenterAlignedDownTrig   = 0x02,
//     CenterAlignedUpTrig     = 0x04,
//     CenterAlignedDualTrig   = 0x06
// };

enum class [[nodiscard]] TimerCountMode:uint8_t{
    Up                      = 0x0000,
    Down                    = 0x0010,
    CenterAlignedDownTrig   = 0x0020,
    CenterAlignedUpTrig     = 0x0040,
    CenterAlignedDualTrig   = 0x0060
};


struct [[nodiscard]] TimerChannelSelection{
    enum class [[nodiscard]] Kind:uint8_t{
        CH1     =   0b000,
        CH1N    =   0b001,
        CH2     =   0b010,
        CH2N    =   0b011,
        CH3     =   0b100,
        CH3N    =   0b101,
        CH4     =   0b110
    };

    using enum Kind;

    TimerChannelSelection(const TimerChannelSelection::Kind kind):
        kind_(kind){}

    [[nodiscard]] constexpr Kind kind() const{ return kind_; }

    [[nodiscard]] size_t index() const {
        return (std::bit_cast<uint8_t>(kind_) >> 1);
    }

    [[nodiscard]] bool is_co() const {
        return (std::bit_cast<uint8_t>(kind_)) & 0x01;
    }
private:
    Kind kind_;
};




enum class [[nodiscard]] TimerTrgoSource:uint8_t{
    Reset   = 0x0000,             
    Enable  = 0x0010,           
    Update  = 0x0020,           
    OC1     = 0x0030   ,            
    OC1R    = 0x0040,            
    OC2R    = 0x0050,            
    OC3R    = 0x0060,            
    OC4R    = 0x0070            
};


// enum class [[nodiscard]] TimerTrgoSource:uint8_t{
//     Reset   = 0x000,             
//     Enable  = 0x001,           
//     Update  = 0x002,           
//     OC1     = 0x003,            
//     OC1R    = 0x004,            
//     OC2R    = 0x005,            
//     OC3R    = 0x006,            
//     OC4R    = 0x007            
// };


enum class [[nodiscard]] TimerIT:uint8_t{
    Update  = 0x0001,
    CC1     = 0x0002,
    CC2     = 0x0004,
    CC3     = 0x0008,
    CC4     = 0x0010,
    COM     = 0x0020,
    Trigger = 0x0040,
    Break   = 0x0080,
};

// enum class [[nodiscard]] TimerIT:uint8_t{
//     Update  = 0x01,
//     CC1     = 0x02,
//     CC2     = 0x04,
//     CC3     = 0x08,
//     CC4     = 0x10,
//     COM     = 0x20,
//     Trigger = 0x40,
//     Break   = 0x80,
// };

enum class [[nodiscard]] TimerBdtrLockLevel:uint16_t{
    Off     = 0x0000,
    Low     = 0x0100,
    Medium  = 0x0200,
    High    = 0x0300
};

// enum class [[nodiscard]] TimerBdtrLockLevel:uint8_t{
//     Off     = 0x00,
//     Low     = 0x01,
//     Medium  = 0x02,
//     High    = 0x03
// };

enum class [[nodiscard]] TimerOcMode:uint8_t{
    Freeze              = 0b000,
    ActiveUnlessCvr     = 0b001,
    InactiveUnlessCvr   = 0b010,
    ToggleWhenCvr       = 0b011,
    AlwaysInactive     = 0b100,
    AlwaysActive       = 0b101,
    ActiveBelowCvr     = 0b110,
    ActiveAboveCvr      = 0b111,
};

using TimerEvent = TimerIT;
}