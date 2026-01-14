#pragma once

#include "core/utils/sumtype.hpp"
#include "primitive/nearest_freq.hpp"

namespace ymd{
class OutputStream;
}

namespace ymd::hal::timer{

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
        return CLAMP(int(target_div) / (int(arr) + 1) - 1, 0, 0xFFFF);
    };

    [[maybe_unused]]
    auto calc_arr_from_psc = [target_div](const uint16_t psc) -> uint16_t {
        return CLAMP(int(target_div) / (int(psc) + 1) - 1, 0, 0xFFFF);
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

        for(int psc = expect_psc - 2; psc < expect_psc + 2; psc++){
            const auto freq = calc_freq_from_arr_and_psc(arr, psc);
            if(last_freq_.has_value()){
                if((last_freq_.value() - count_freq) * (freq - count_freq) < 0) break;
            }else{
                last_freq_ = freq;
            }
            const auto freq_err = uint32_t(ABS(int(freq) - int(count_freq)));
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

    friend OutputStream & operator <<(OutputStream & os, const Self & self){
        return os << os.field("arr")(self.arr) << os.splitter() 
            << os.field("psc")(self.psc);
    }
};


struct [[nodiscard]] CountFreq:
    public Sumtype<ArrAndPsc, NearestFreq>{
};


struct [[nodiscard]] CountMode{
    enum class [[nodiscard]] Kind:uint8_t{
        //b0:计数器方向：
        // 0：计数器的计数模式为增计数；
        // 1：计数器的计数模式为减计数。
        // 注：当计数器配置为中央对齐模式或编码器模式时，
        // 该位无效。
        // b1~b2:
        // 中央对齐模式选择：
        // 00：边沿对齐模式。计数器依据方向位(DIR)向上或
        // 向下计数。
        // 01：中央对齐模式 1。计数器交替地向上和向下计
        // 数。配置为输出的通道(CHCTLRx 寄存器中 CCxS=00)
        // 的输出比较中断标志位，只在计数器向下计数时被
        // 设置。
        // 10：中央对齐模式 2。计数器交替地向上和向下计
        // 数。配置为输出的通道(CHCTLRx 寄存器中 CCxS=00)
        // 的输出比较中断标志位，只在计数器向上计数时被
        // 设置。
        // 11：中央对齐模式 3。计数器交替地向上和向下计
        // 数。配置为输出的通道(CHCTLRx 寄存器中 CCxS=00)
        // 的输出比较中断标志位，在计数器向上和向下计数
        // 时均被设置。
        // 注：在计数器使能时(CEN=1)，不允许从边沿对齐模
        // 式转换到中央对齐模式。
        Up                      = 0b000,
        Down                    = 0b001,
        CenterAlignedDownTrig   = 0b010,
        CenterAlignedUpTrig     = 0b100,
        CenterAlignedDualTrig   = 0b110
    };

    using enum Kind;

    constexpr CountMode(Kind kind):kind_(kind){}
    constexpr CountMode(const CountMode &) = default;
    [[nodiscard]] constexpr Kind kind() const {return kind_;}

    [[nodiscard]] constexpr uint32_t to_bits() const {return static_cast<uint32_t>(kind_);}

    [[nodiscard]] constexpr bool is_center_aligned() const {
        switch(kind()){
            case CenterAlignedDownTrig:
            case CenterAlignedUpTrig:
            case CenterAlignedDualTrig:
                return true;
            default:
                return false;
        }
    }


private:
    Kind kind_;
};


struct [[nodiscard]] ChannelSelection{
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

    constexpr ChannelSelection(const Kind kind):
        kind_(kind){}

    [[nodiscard]] constexpr Kind kind() const{ return kind_; }

    [[nodiscard]] constexpr size_t index() const {
        return (std::bit_cast<uint8_t>(kind_) >> 1);
    }

    [[nodiscard]] constexpr bool is_co() const {
        return (std::bit_cast<uint8_t>(kind_)) & 0x01;
    }
private:
    Kind kind_;
};


enum class [[nodiscard]] SlaveMode:uint8_t{
    Reset = 0x04,
    Gated = 0x05,
    Trigger = 0x06,
    External1 = 0x07
};

enum class [[nodiscard]] TrgoSource:uint8_t{
    Reset   = 0x00,             
    Enable  = 0x01,           
    Update  = 0x02,           
    OC1     = 0x03,            
    OC1R    = 0x04,            
    OC2R    = 0x05,            
    OC3R    = 0x06,            
    OC4R    = 0x07            
};

enum class [[nodiscard]] TrgiSource:uint8_t{
    ITR0   = 0x00,             
    ITR1   = 0x01,
    ITR2   = 0x02,
    ITR3   = 0x03,
    TI1F_ED = 0x04,
    TI1FP1 = 0x05,
    TI2FP2 = 0x06,
    ETRF   = 0x07
};

enum class [[nodiscard]] DmaSource:uint8_t{
    Update = 0x01,
    CC1     = 0x02,
    CC2     = 0x04,
    CC3     = 0x08,
    CC4     = 0x10,
    COM     = 0x20,
    Trigger = 0x40,
};

enum class [[nodiscard]] ExternalClkSource:uint8_t{
    TI1 = 0x0050,
    TI2 = 0x0060,
    TI1ED = 0x0040,
};

enum class [[nodiscard]] IT:uint16_t{
    Update  = 0x0001,
    CC1     = 0x0002,
    CC2     = 0x0004,
    CC3     = 0x0008,
    CC4     = 0x0010,
    COM     = 0x0020,
    Trigger = 0x0040,
    Break   = 0x0080,
};

enum class [[nodiscard]] BdtrLockLevel:uint8_t{
    Off     = 0x00,
    Low     = 0x01,
    Medium  = 0x02,
    High    = 0x03
};

enum class [[nodiscard]] OcMode:uint8_t{
    Freeze              = 0b000,
    ActiveUnlessCvr     = 0b001,
    InactiveUnlessCvr   = 0b010,
    ToggleWhenCvr       = 0b011,
    AlwaysInactive     = 0b100,
    AlwaysActive       = 0b101,
    ActiveBelowCvr     = 0b110,
    ActiveAboveCvr      = 0b111,
};

using Event = IT;

struct [[nodiscard]] DeadzoneCode{
    using Self = DeadzoneCode;
    uint8_t bits;

    static constexpr DeadzoneCode from_ns(
        const uint32_t aligned_bus_clk_freq, 
        const ymd::Nanoseconds ns
    ){
        const auto bits = [&] -> uint8_t{
            const auto clk_freq_mhz = (aligned_bus_clk_freq / 1000000);
            const uint16_t scale = ((static_cast<uint32_t>(ns.count()) * clk_freq_mhz) / 1000);
            if(scale < 128){
                return scale;
            }else if(scale < 256){
                constexpr uint8_t head = 0b10000000;
                constexpr uint8_t mask = 0b00111111;

                return static_cast<uint8_t>(((((MIN(scale, 254) >> 1) - 64) & mask) | head));
            }else if(scale < 509){
                constexpr uint8_t head = 0b11000000;
                constexpr uint8_t mask = 0b00011111;

                return static_cast<uint8_t>(((((MIN(scale, 504) >> 1) - 32) & mask) | head));
            }else if(scale < 1009){
                constexpr uint8_t head = 0b11100000;
                constexpr uint8_t mask = 0b00011111;

                return static_cast<uint8_t>((((MIN(scale, 1008) >> 4) - 32) & mask) | head);
            }else{
                return static_cast<uint8_t>(0xff);
            }
        }();

        return DeadzoneCode{bits};
    }
};
}

namespace ymd::hal{
using TimerCountMode = timer::CountMode;
using TimerCountFreq = timer::CountFreq;
using TimerDeadzoneCode = timer::DeadzoneCode;
using TimerIT = timer::IT;
using TimerEvent = timer::Event;
using TimerTrgoSource = timer::TrgoSource;
using TimerTrgiSource = timer::TrgiSource;
using TimerSlaveMode = timer::SlaveMode;
using TimerOcMode = timer::OcMode;
using TimerChannelSelection = timer::ChannelSelection;
using TimerBdtrLockLevel = timer::BdtrLockLevel;
}