#pragma once


#include "core/utils/Option.hpp"
#include "core/utils/sumtype.hpp"
#include "primitive/arithmetic/percentage.hpp"
#include <memory>
#include <functional>

namespace ymd{
    class OutputStream;
}
namespace ymd::hal::can{


/// @brief CAN Swj时间长度
enum class [[nodiscard]] Swj:uint8_t{
    _1tq = 0x00,
    _2tq = 0x01,
    _3tq = 0x02,
    _4tq = 0x03
};

/// @brief CAN Bs1时间长度
enum class [[nodiscard]] Bs1:uint8_t{
    _1tq = 0x00,
    _2tq = 0x01,
    _3tq = 0x02,
    _4tq = 0x03,
    _5tq = 0x04,
    _6tq = 0x05,
    _7tq = 0x06,
    _8tq = 0x07,
    _9tq = 0x08,
    _10tq = 0x09,
    _11tq = 0x0A,
    _12tq = 0x0B,
    _13tq = 0x0C,
    _14tq = 0x0D,
    _15tq = 0x0E,
    _16tq = 0x0F
};


/// @brief CAN Bs2时间长度
enum class [[nodiscard]] Bs2:uint8_t{
    _1tq = 0x00,
    _2tq = 0x01,
    _3tq = 0x02,
    _4tq = 0x03,
    _5tq = 0x04,
    _6tq = 0x05,
    _7tq = 0x06,
    _8tq = 0x07,
};

struct [[nodiscard]] NominalBitTimmingCoeffs{
    uint16_t prescale;
    Swj swj;
    Bs1 bs1;
    Bs2 bs2;

    [[nodiscard]] static constexpr NominalBitTimmingCoeffs from(
        const uint32_t aligned_bus_clk_freq,
        const uint32_t baud_freq,
        const Percentage<uint8_t> sample_point
    ){
        //works only at 144mhz pclk1 freq
        //TODO : support other freq
        if(aligned_bus_clk_freq != 144'000'000) 
            __builtin_trap();

        [[maybe_unused]] const auto sample_percents = sample_point.percents();

        switch(baud_freq){
            case 10'000:    return {.prescale = 900,  .swj = Swj::_1tq,     
                .bs1 = Bs1::_12tq,     .bs2 = Bs2::_3tq};
            case 20'000:    return {.prescale = 450,  .swj = Swj::_1tq,     
                .bs1 = Bs1::_12tq,     .bs2 = Bs2::_3tq};
            case 50'000:    return {.prescale = 180,  .swj = Swj::_1tq,     
                .bs1 = Bs1::_12tq,     .bs2 = Bs2::_3tq};
            case 100'000:   return {.prescale = 90,   .swj = Swj::_1tq,     
                .bs1 = Bs1::_12tq,     .bs2 = Bs2::_3tq};
            case 125'000:   return {.prescale = 72,   .swj = Swj::_1tq,     
                .bs1 = Bs1::_12tq,     .bs2 = Bs2::_3tq};
            case 250'000:   return {.prescale = 36,   .swj = Swj::_1tq,     
                .bs1 = Bs1::_12tq,     .bs2 = Bs2::_3tq};
            case 500'000:   return {.prescale = 18,   .swj = Swj::_1tq,     
                .bs1 = Bs1::_12tq,     .bs2 = Bs2::_3tq};

            case 800'000:       return {.prescale = 9,    .swj = Swj::_2tq,     
                .bs1 = Bs1::_15tq,     .bs2 = Bs2::_4tq};
            case 1000'000:      return {.prescale = 9,    .swj = Swj::_2tq,     
                .bs1 = Bs1::_12tq,     .bs2 = Bs2::_3tq};
        };

        __builtin_trap();
    }
};



enum class [[nodiscard]] FifoIndex:uint8_t{
    _0 = 0,
    _1 = 1
};

enum class [[nodiscard]] MailboxIndex:uint8_t{
    _0 = 0,
    _1 = 1,
    _2 = 2
};

enum class [[nodiscard]] InterruptFlagBit:uint16_t{
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



struct [[nodiscard]] Tq{
    using Self = Tq;
    static constexpr Self from_count(const uint8_t count){
        if(count == 0) __builtin_abort();
        return Self{static_cast<uint8_t>(count - 1)};
    }

    static constexpr Self from_bits(const uint8_t bits){
        return Self{static_cast<uint8_t>(bits)};
    }

    [[nodiscard]] constexpr uint8_t to_bits() const {return bits_;}
    [[nodiscard]] constexpr uint8_t count() const {return static_cast<uint8_t>(bits_ + 1);}

private:
    uint8_t bits_;

    constexpr Tq(const uint8_t bits):bits_{bits}{}
};

class [[nodiscard]] Baudrate{
public:
    static constexpr Percentage<uint8_t> DEFAULT_SAMPLE_POINT = 
        Percentage<uint8_t>::from_percents(80).unwrap();
    enum class [[nodiscard]] Kind:uint8_t{
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
        Percentage<uint8_t> sample_point = DEFAULT_SAMPLE_POINT
    ):
        freq_(kind2freq(kind)),
        sample_point_(sample_point){}
    constexpr Baudrate(
        const uint32_t ferq, 
        const Percentage<uint8_t> sample_point = DEFAULT_SAMPLE_POINT
    ):
        freq_(ferq),
        sample_point_(sample_point){}

    constexpr NominalBitTimmingCoeffs to_coeffs(
        const uint32_t aligned_bus_clk_freq,
        const Percentage<uint8_t> sample_point = DEFAULT_SAMPLE_POINT
    ) const {
        return NominalBitTimmingCoeffs::from(
            aligned_bus_clk_freq, 
            freq_, 
            sample_point
        );
    }

    [[nodiscard]] constexpr bool has_same_freq(const Baudrate& rhs) const {
        return freq_ == rhs.freq_;
    }

    [[nodiscard]] constexpr bool has_same_freq(const Kind kind) const {
        return freq_ == kind2freq(kind);
    }

    [[nodiscard]] constexpr uint32_t freq() const {return freq_;}

    [[nodiscard]] constexpr Percentage<uint8_t> sample_point() const {
        return sample_point_;}

private:
    uint32_t freq_;
    Percentage<uint8_t> sample_point_;
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
        return os << os.field("baudrate")(self.freq_) << os.splitter() <<
            os.field("sample_point")(self.sample_point_.percents(), '%')
        ;

        __builtin_unreachable();
    }
};



struct [[nodiscard]] WiringMode{
    using Self = WiringMode;

    enum class [[nodiscard]] Kind:uint8_t {
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

struct [[nodiscard]] ErrorFlags{
    uint32_t error_warning:1;
    uint32_t error_passive:1;
    uint32_t bus_off:1;
    uint32_t stuff:1;
    uint32_t form:1;
    uint32_t acknowledgment:1;
    uint32_t bit_recessive:1;
    uint32_t bit_dominant:1;
    uint32_t crc_error:1;
    uint32_t fifo0_overrun:1;
    uint32_t fifo1_overrun:1;

    uint32_t mbox0_arbitration_lost:1;
    uint32_t mbox0_transmit:1;
    uint32_t mbox1_arbitration_lost:1;
    uint32_t mbox1_transmit:1;
    uint32_t mbox2_arbitration_lost:1;
    uint32_t mbox2_transmit:1;

    uint32_t timout:1;
    uint32_t peripheral_not_initialized:1;
    uint32_t peripheral_not_ready:1;
    uint32_t peripheral_not_started:1;
};

OutputStream & operator<<(OutputStream & os, const Error & fault);

enum class [[nodiscard]] LibError:uint8_t{
    BlockingTransmitTimeout,
    NoMailboxAvailable,
    SoftFifoOverflow
};

OutputStream & operator<<(OutputStream & os, const LibError & error);

enum class [[nodiscard]] RtrSpecfier:uint8_t{
    Discard,
    RemoteOnly,
    DataOnly
};

enum class [[nodiscard]] Rtr:uint8_t{
    Data = 0,
    Remote = 1,
};  

struct [[nodiscard]] NominalBitTimming:
    public Sumtype<NominalBitTimmingCoeffs, Baudrate>{
};

// https://docs.rs/embassy-stm32/latest/embassy_stm32/can/config/enum.TxBufferMode.html
enum class TxBufferMode:uint8_t {
    // TX FIFO operation - In this mode CAN frames are trasmitted strictly in write order.
    Fifo,
    // TX priority queue operation - In this mode CAN frames are transmitted according to CAN priority.
    Priority,
};

#if 0
struct FdCanConfig final {
    // Nominal Bit Timings
    CanNominalBitTimming nbtr;
    // (Variable) Data Bit Timings
    DataBitTiming dbtr;
    // Enables or disables automatic retransmission of messages

    // If this is enabled, the CAN peripheral will automatically try to retransmit each frame util it can be sent. Otherwise, it will try only once to send each frame.

    // Automatic retransmission is enabled by default.
    Enable automatic_retransmit;

    // The transmit pause feature is intended for use in CAN systems where the CAN message 
    // identifiers are permanently specified to specific values and cannot easily be changed.

    // These message identifiers can have a higher CAN arbitration priority than other defined messages, 
    // while in a specific application their relative arbitration priority must be inverse.

    // This may lead to a case where one ECU sends a burst of CAN messages that cause another ECU CAN 
    // messages to be delayed because that other messages have a lower CAN arbitration priori
    Enable transmit_pause;

    // Enabled or disables the pausing between transmissions
    // This feature looses up burst transmissions coming from a single node and it protects against 
    // “babbling idiot” scenarios where the application program erroneously requests too many transmissions.
    FrameTransmissionConfig frame_transmit;

    // Non Isoe Mode If this is set, the FDCAN uses the CAN FD frame format as specified by the 
    // Bosch CAN FD Specification V1.0.
    Enable non_iso_mode;

    // Edge Filtering: Two consecutive dominant tq required to detect an edge for hard synchronization
    Enable edge_filtering;

    // Enables protocol Error handling
    Enable protocol_Error_handling;

    // Sets the general clock divider for this FdCAN instance
    ClockDivider clock_divider;

    // Sets the timestamp source
    TimestampSource timestamp_source;

    // Configures the Global Filter
    GlobalFilter global_filter;

    // TX buffer mode (FIFO or priority queue)
    TxBufferMode tx_buffer_mode;
};
#endif
};

namespace ymd::hal{
using CanSwj = can::Swj;
using CanBaudrate = can::Baudrate;
using CanWiringMode = can::WiringMode;
using CanError = can::Error;
using CanLibError = can::LibError;
using CanRtrSpecfier = can::RtrSpecfier;
using CanRtr = can::Rtr;
using CanNominalBitTimming = can::NominalBitTimming;
using CanNominalBitTimmingCoeffs = can::NominalBitTimmingCoeffs;
using CanMailboxIndex = can::MailboxIndex;
using CanFifoIndex = can::FifoIndex;
using CanIT = can::InterruptFlagBit;
}

