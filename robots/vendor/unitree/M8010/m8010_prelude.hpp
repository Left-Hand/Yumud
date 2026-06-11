#pragma once

#include "core/stream/ostream.hpp"
#include "core/utils/sumtype.hpp"
#include "core/utils/bytes/buffer_cursor.hpp"
#include "core/container/bits_set.hpp"
#include "core/utils/bits/bits_caster.hpp"

#include "core/utils/Option.hpp"
#include "core/utils/Result.hpp"

#include "primitive/can/can_frame.hpp"
#include "primitive/arithmetic/angular.hpp"


namespace ymd::robots::unitree::m8010{


static constexpr size_t TX_FRAME_SIZE = 17;
static constexpr size_t RX_FRAME_SIZE = 16;

struct [[nodiscard]] MotorId final{
    uint8_t bits;

    [[nodiscard]] constexpr bool is_valid() const noexcept {
        return (bits & 0xf0) == 0;
    }

    [[nodiscard]] constexpr bool is_boardcast() const noexcept {
        return (bits & 0x0f) == 0x0f;
    }

    [[nodiscard]] constexpr uint8_t count() const noexcept {
        return bits & 0x0f;
    }
};

// 为了保证标定效果，切换到编码器校准模式后，需要等待5s再进行通信 
// （期间不可以给电机发送任何数据包，否则会标定失败
struct [[nodiscard]] WorkingMode final{
    using Self = WorkingMode;
    enum class [[nodiscard]] Kind  : uint8_t{
        Default = 0,
        Closeloop = 1,
        Calibrating = 2,
        Reserved = 3,
    };

    uint8_t bits;

    static constexpr Option<Self> try_from_u8(const uint8_t b){
        if(b >= static_cast<uint8_t>(Kind::Reserved)) return None;
        return Some(Self{b});
    }

    static constexpr Self from_default(){
        return Self{static_cast<uint8_t>(Kind::Default)};
    }

    [[nodiscard]] constexpr bool is_calibrating() const noexcept {
        return bits == static_cast<uint8_t>(Kind::Calibrating);
    }

    [[nodiscard]] constexpr bool is_closeloop() const noexcept {
        return bits == static_cast<uint8_t>(Kind::Closeloop);
    }

    [[nodiscard]] constexpr bool is_default() const noexcept {
        return bits == static_cast<uint8_t>(Kind::Default);
    }
};


struct [[nodiscard]] ModeInfo final{
    uint8_t bits;

    constexpr MotorId motor_id() const noexcept {
        return MotorId(bits & 0x0f);
    }

    constexpr WorkingMode mode() const noexcept {
        return WorkingMode((bits >> 4) & 0x07);
    }
};

struct [[nodiscard]] TorqueCode final{
    using Self = TorqueCode;
    int16_t bits;

    static constexpr Result<Self, std::strong_ordering> 
    try_from_nm(const iq16 torque_nm){
        const auto ret = torque_nm << 8;
        if(ret > std::numeric_limits<int16_t>::max()) 
            return Err(std::strong_ordering::greater);
        if(ret < std::numeric_limits<int16_t>::min()) 
            return Err(std::strong_ordering::less);
        return Ok(Self{round_cast<int16_t>(ret)});
    }
};



struct [[nodiscard]] X2Code final{
    using Self = X2Code;
    int16_t bits;

    static constexpr Result<Self, std::strong_ordering> 
    try_from_speed(const Angular<iq16> speed){
        const auto ret = speed.to_turns() << 8;
        if(ret > std::numeric_limits<int16_t>::max()) 
            return Err(std::strong_ordering::greater);
        if(ret < std::numeric_limits<int16_t>::min()) 
            return Err(std::strong_ordering::less);
        return Ok(Self{math::round_cast<int16_t>(ret)});
    }
};



struct [[nodiscard]] X1Code final{
    using Self = X1Code;
    int32_t bits;

    static constexpr Result<Self, std::strong_ordering> try_from_turns(const iq15 turns){
        const auto ret = turns.to_bits();
        return Ok(Self{math::round_cast<int32_t>(ret)});
    }
};


struct [[nodiscard]] KpCode final{
    using Self = KpCode;
    uint16_t bits;

    static constexpr auto MAX_BITS_VAL = static_cast<uint16_t>(25.6 * 1280u);

    static constexpr Result<Self, std::strong_ordering> try_from(const uq16 val){
        const auto ret = val * 1280u;
        if(val > uq16(25.6)) 
            return Err(std::strong_ordering::greater);
        return Ok(Self{math::round_cast<uint16_t>(ret)});
    }
};


struct [[nodiscard]] KdCode final{
    using Self = KdCode;
    uint16_t bits;

    static constexpr Result<KdCode, std::strong_ordering> try_from(const uq16 val){
        const auto ret = val * 1280u;
        if(val > uq16(25.6)) 
            return Err(std::strong_ordering::greater);
        return Ok(Self{math::round_cast<uint16_t>(ret)});
    }
};


struct [[nodiscard]] TempCode final{
    uint8_t bits;

    using Self = TempCode;

    constexpr int8_t to_celeius() const noexcept {
        return std::bit_cast<uint8_t>(bits);
    }

    constexpr Self from_bits(uint8_t b) const noexcept {
        return Self{.bits = std::bit_cast<uint8_t>(b)};
    }

    constexpr void fill_bytes(std::span<uint8_t, 1> bytes) const noexcept {
        bytes[0] = std::bit_cast<uint8_t>(bits);
    }
};

struct [[nodiscard]] ErrorCode final{ 
    using Self = ErrorCode;
    enum class Kind :uint8_t{
        Ok = 0,
        OverHeat = 1,
        OverCurrent = 2,
        OverVoltage = 3,
        Encoder = 4
    };

    uint8_t bits;

    [[nodiscard]] constexpr bool is_ok() const noexcept {
        return bits == 0;
    }

    [[nodiscard]] constexpr bool is_err() const noexcept {return not is_ok();}

    [[nodiscard]] constexpr bool is_invalid() const noexcept {return bits > 4;}

    [[nodiscard]] constexpr Kind unwrap_err() const noexcept {
        if(is_ok()) [[unlikely]] __builtin_trap();
        return static_cast<Kind>(bits);
    }
};


struct [[nodiscard]] RxMisc final{
    uint16_t err_bits : 3;
    uint16_t force_bits : 12;
    uint16_t __resv__:1;

    static constexpr RxMisc from_bytes(std::span<const uint8_t, 2> bytes){
        const uint16_t ret = bytes[0] | (bytes[1] << 8);
        return std::bit_cast<RxMisc>(ret);
    }

    constexpr uint16_t to_u16() const {
        return std::bit_cast<uint16_t>(*this);
    }
};

}