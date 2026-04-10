#pragma once


#include "core/utils/Option.hpp"
#include "core/utils/sumtype.hpp"
#include "primitive/arithmetic/percentage.hpp"
#include <memory>
#include <functional>


namespace ymd{
    struct OutputStream;
}


namespace ymd::hal::can{

enum struct [[nodiscard]] LibError:uint8_t{
    NoMailboxAvailable,
    TxQueueFull,
    TxQueueOverflow,
};


struct [[nodiscard]] Tq final{

    using Self = Tq;
    static constexpr Self from_num(const uint8_t count){
        if(count == 0) __builtin_abort();
        return std::bit_cast<Self>(static_cast<uint8_t>(count - 1));
    }

    static constexpr Self from_bits(const uint8_t bits){
        return std::bit_cast<Self>(static_cast<uint8_t>(bits));
    }

    constexpr bool operator ==(const Self& other) const {return bits == other.bits;}
    [[nodiscard]] constexpr uint8_t to_num() const {return bits + 1;}

    [[nodiscard]] constexpr uint8_t to_bits() const {return bits;}
private:
    uint8_t bits;
};


// 标称比特率
struct [[nodiscard]] NominalBitTimmingCoeffs final{
    // ・Set the sample point to be approximately between 60% and 80% of the bit time.
    //     In most cases, a setting between 75% and 80% is recommended.

    // ・It is recommended to set the synchronization jump width (SJW) and time segment 2 (TSeg2) to the same value.

    // ・For specific parameter setting examples, please refer to the [Parameter Setting Example with Sample Point Position] topic.

    // CiA（CAN in Automation）推荐：87.5% 作为默认值（对应 BS1=13, BS2=2, 总 TQ=16）。
    // 工业常用值：75% ~ 80% 以兼顾延迟和时钟容差。

    // 对于低精度 RC 振荡器（±1% ~ ±2%）
    // 适当提前采样点（75% 左右）并增大 SJW 是更稳健的策略，既能容忍时钟漂移，又不会因信号延迟而导致采样错误
    // 可配置采样点 75%~80%，SJW 取 2~4 TQ，并确保 BS2 ≥ SJW。

    // 常用配置表
    // https://help.contec.com/pc-helper/api-tool-wdm/en/mergedProjects/CCAN/faq/sampling_point/sample_point_for_can_4pf_pe.htm

    // 各时序段说明
    // https://help.contec.com/pc-helper/api-tool-lnx/en/mergedProjects/ccan/glossary/norminal_bit_time.htm

    // http://www.bittiming.can-wiki.info/
    // https://blog.csdn.net/q601785959/article/details/105312419


    using Self = NominalBitTimmingCoeffs;
    uint16_t prescale;
    Tq swj;
    Tq bs1;
    Tq bs2;

    // [[nodiscard]] constexpr bool operator ==(const NominalBitTimmingCoeffs &) = default;
    [[nodiscard]] constexpr uint32_t calc_baudrate_hz(
        const uint32_t aligned_bus_clk_freq_hz
    ) const {
        uint32_t total_tq = 1 + bs1.to_num() + bs2.to_num();
        return aligned_bus_clk_freq_hz / (prescale * total_tq);
    }

    [[nodiscard]] constexpr Percentage<uint8_t> calc_sample_point() const {
        const uint8_t bs1_num = bs1.to_num();
        const uint8_t bs2_num = bs2.to_num();
        const uint8_t total_tq = 1 + bs1_num + bs2_num;
        const uint8_t sample_tq = 1 + bs1_num;  // 采样点位于同步段结束后
        // 四舍五入计算百分比
        const uint8_t percent = (sample_tq * 100 + total_tq / 2) / total_tq;
        return Percentage<uint8_t>::from_percents_unchecked(percent);
    }

    [[nodiscard]] static constexpr Option<NominalBitTimmingCoeffs> try_from(
        const uint32_t aligned_bus_clk_freq_hz,
        const uint32_t baud_freq_hz,
        const Percentage<uint8_t> sample_point
    ){
    const auto sample_percents = sample_point.percents();

        // 尝试不同的 TQ 总数（典型范围为 8~25）
        for(uint8_t ntq = 25; ntq >= 8; --ntq){
            // 计算分频系数（整数除法）
            const uint32_t denominator = static_cast<uint32_t>(baud_freq_hz) * ntq;
            if (denominator == 0) continue;                     // 防御性检查
            const uint32_t prescale_calc = aligned_bus_clk_freq_hz / denominator;

            // 分频系数必须在有效范围内（常见 CAN 控制器为 1~1024）
            if(prescale_calc == 0 || prescale_calc > 1024) continue;
            // 验证是否能精确整除
            if(aligned_bus_clk_freq_hz != prescale_calc * denominator) continue;

            // 同步段固定为 1 TQ
            const uint8_t sync_seg_tq = 1;

            // 计算采样点所在的 TQ 位置（四舍五入）
            const uint8_t sample_tq = (ntq * sample_percents + 50) / 100;

            // 采样点不能落在同步段内，也不能超出 NTQ
            if(sample_tq <= sync_seg_tq || sample_tq >= ntq) continue;

            // 分配 BS1 和 BS2
            const uint8_t bs1_num = sample_tq - sync_seg_tq;     // BS1 = 采样点 - 同步段
            const uint8_t bs2_num = ntq - sample_tq;             // BS2 = NTQ - 采样点

            // 检查 BS1 和 BS2 是否在常见控制器允许的范围内
            if(bs1_num < 1 || bs1_num > 16 || bs2_num < 1 || bs2_num > 8) continue;

            // 确定同步跳转宽度（SJW）
            uint8_t swj_tq = (ntq >= 20) ? 2 : 1;
            if (swj_tq > bs2_num) {
                swj_tq = bs2_num;      // 不能超过 BS2
            }

            // 所有参数有效，返回结果
            return Some(NominalBitTimmingCoeffs{
                .prescale = static_cast<uint16_t>(prescale_calc),
                .swj = Tq::from_num(swj_tq),
                .bs1 = Tq::from_num(bs1_num),
                .bs2 = Tq::from_num(bs2_num),
            });
        }

        // 未找到可行配置
        return None;
    }
};





enum struct [[nodiscard]] FifoIndex:uint8_t{
    _0 = 0,
    _1 = 1
};

enum struct [[nodiscard]] MailboxIndex:uint8_t{
    _0 = 0,
    _1 = 1,
    _2 = 2
};

enum struct [[nodiscard]] InterruptFlagBit:uint16_t{
    TME = (1u << 0),

    FMP0 = (1u << 1),
    FF0 = (1u << 2),
    FOV0 = (1u << 3),
    FMP1 = (1u << 4),
    FF1 = (1u << 5),
    FOV1 = (1u << 6),

    WKU = (1u << 7),
    SLK = (1u << 8),

    EWG = (1u << 9),
    EPV = (1u << 10),
    BOF = (1u << 11),
    LEC = (1u << 12),
    ERR = (1u << 13),
};


//can错误
enum class [[nodiscard]] Error:uint8_t{
    Stuff = 0x1,
    Form = 0x2,
    Acknowledge = 0x3,
    BitRecessive = 0x4,
    BitDominant = 0x5,
    Crc = 0x6,
    SoftwareSet = 0x7,
};


OutputStream & operator<<(OutputStream & os, const Error & error);

struct [[nodiscard]] Baudrate final{
public:
    static constexpr Percentage<uint8_t> DEFAULT_SAMPLE_POINT = 
        Percentage<uint8_t>::from_percents(80).unwrap();

    uint32_t bitrate_hz;
    Percentage<uint8_t> sample_point;

    enum struct [[nodiscard]] Kind:uint8_t{
        _10K,
        _20K,
        _50K,
        _100K,
        _125K,
        _250K,
        _500K,
        _800K,
        _1M
    };

    using enum Kind;
    constexpr Baudrate(
        Kind kind, 
        Percentage<uint8_t> _sample_point = DEFAULT_SAMPLE_POINT
    ):
        bitrate_hz(kind2freq(kind)),
        sample_point(_sample_point){}
    constexpr Baudrate(
        const uint32_t ferq, 
        const Percentage<uint8_t> _sample_point = DEFAULT_SAMPLE_POINT
    ):
        bitrate_hz(ferq),
        sample_point(_sample_point){}

    constexpr Option<NominalBitTimmingCoeffs> try_into_coeffs(
        const uint32_t aligned_bus_clk_freq_hz,
        const Percentage<uint8_t> _sample_point = DEFAULT_SAMPLE_POINT
    ) const {
        return NominalBitTimmingCoeffs::try_from(
            aligned_bus_clk_freq_hz, 
            bitrate_hz, 
            _sample_point
        );
    }

    [[nodiscard]] constexpr bool has_same_freq(const Baudrate& rhs) const {
        return bitrate_hz == rhs.bitrate_hz;
    }

    [[nodiscard]] constexpr bool has_same_freq(const Kind kind) const {
        return bitrate_hz == kind2freq(kind);
    }

    [[nodiscard]] constexpr uint32_t freq() const {return bitrate_hz;}
private:

    static constexpr uint32_t kind2freq(const Kind kind) {
        switch(kind){
            case Kind::_10K: return 10'000;
            case Kind::_20K: return 20'000;
            case Kind::_50K: return 50'000;
            case Kind::_100K: return 100'000;
            case Kind::_125K: return 125'000;
            case Kind::_250K: return 250'000;
            case Kind::_500K: return 500'000;
            case Kind::_800K: return 800'000;
            case Kind::_1M: return 1000'000;
        }
        __builtin_trap();
    }

    [[nodiscard]] static constexpr Option<Kind> baud2kind(uint32_t freq){
        switch(freq){
            case 10'000: return Some(Kind::_10K);
            case 20'000: return Some(Kind::_20K);
            case 50'000: return Some(Kind::_50K);
            case 100'000: return Some(Kind::_100K);
            case 125'000: return Some(Kind::_125K);
            case 250'000: return Some(Kind::_250K);
            case 500'000: return Some(Kind::_500K);
            case 800'000: return Some(Kind::_800K);
            case 1000'000: return Some(Kind::_1M);
        }
        return None;
    }

    friend OutputStream & operator<<(OutputStream & os, const Baudrate & self){
        return os << os.field("baudrate")(self.bitrate_hz) << os.splitter() <<
            os.field("sample_point")(self.sample_point.percents(), '%')
        ;

        __builtin_unreachable();
    }
};

struct [[nodiscard]] WiringMode final{
    using Self = WiringMode;

    enum struct [[nodiscard]] Kind:uint8_t {
        Normal = 0b00,
        Silent = 0b10,
        SilentLoopback = 0b11,
        Loopback = 0b01
    };

    constexpr WiringMode(Kind kind):kind_(kind){}
    [[nodiscard]] constexpr Kind kind() const{return kind_;}

    [[nodiscard]] constexpr bool is_loopback() const{
        switch(kind_){
            case Kind::Normal: return false;
            case Kind::Silent: return false;
            case Kind::SilentLoopback: return true;
            case Kind::Loopback: return true; 
            default:__builtin_unreachable();
        }
    }
    [[nodiscard]] constexpr bool is_slient() const {
        switch(kind_){
            case Kind::Normal: return false;
            case Kind::Silent: return true;
            case Kind::SilentLoopback: return true;
            case Kind::Loopback: return false; 
            default:__builtin_unreachable();
        }
    }

    using enum Kind;
private:
    Kind kind_;
};



OutputStream & operator<<(OutputStream & os, const LibError & error);

enum struct [[nodiscard]] RtrSpecfier:uint8_t{
    Discard,
    RemoteOnly,
    DataOnly
};

enum struct [[nodiscard]] Rtr:uint8_t{
    Data = 0,
    Remote = 1,
};  

struct [[nodiscard]] NominalBitTimming:
    public Sumtype<NominalBitTimmingCoeffs, Baudrate>{
};

// https://docs.rs/embassy-stm32/latest/embassy_stm32/can/config/enum.TxBufferMode.html
enum struct TxBufferMode:uint8_t {
    // TX FIFO operation - In this mode CAN frames are trasmitted strictly in write order.
    Fifo,
    // TX priority queue operation - In this mode CAN frames are transmitted according to CAN priority.
    Priority,
};
};

namespace ymd::hal{
using CanBaudrate = can::Baudrate;
using CanWiringMode = can::WiringMode;
using CanError = can::Error;
using CanTq = can::Tq;
using CanLibError = can::LibError;
using CanRtrSpecfier = can::RtrSpecfier;
using CanRtr = can::Rtr;
using CanNominalBitTimming = can::NominalBitTimming;
using CanNominalBitTimmingCoeffs = can::NominalBitTimmingCoeffs;
using CanMailboxIndex = can::MailboxIndex;
using CanFifoIndex = can::FifoIndex;
using CanIT = can::InterruptFlagBit;

}

