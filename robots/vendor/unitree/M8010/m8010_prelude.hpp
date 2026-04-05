#pragma once

#include "core/stream/ostream.hpp"
#include "core/utils/sumtype.hpp"
#include "core/container/bits_set.hpp"
#include "core/utils/bits/bits_caster.hpp"

#include "primitive/can/bxcan_frame.hpp"
#include "primitive/arithmetic/angular.hpp"

#include "algo/encrypt/crc_common.hpp"


namespace ymd::robots::unitree{

struct [[nodiscard]] MotorId final{
    uint8_t bits;

    [[nodiscard]] constexpr bool is_valid() const {
        return (bits & 0xf0) == 0;
    }

    [[nodiscard]] constexpr bool is_boardcast() const {
        return (bits & 0x0f) == 0x0f;
    }

    [[nodiscard]] constexpr uint8_t count() const {
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

    [[nodiscard]] constexpr bool is_calibrating() const{
        return bits == static_cast<uint8_t>(Kind::Calibrating);
    }

    [[nodiscard]] constexpr bool is_closeloop() const{
        return bits == static_cast<uint8_t>(Kind::Closeloop);
    }

    [[nodiscard]] constexpr bool is_default() const{
        return bits == static_cast<uint8_t>(Kind::Default);
    }
};


struct [[nodiscard]] ModeInfo final{
    uint8_t bits;

    constexpr MotorId motor_id() const {
        return MotorId(bits & 0x0f);
    }

    constexpr WorkingMode mode() const {
        return WorkingMode((bits >> 4) & 0x07);
    }

    constexpr void fill_bytes(const std::span<uint8_t, 1> bytes) const {
        bytes[0] = bits;
    }
};

struct [[nodiscard]] TorqueCode final{
    using Self = TorqueCode;
    int16_t bits;

    static constexpr Result<Self, std::strong_ordering> try_from_nm(const iq16 torque_nm){
        const auto ret = torque_nm << 8;
        if(ret > std::numeric_limits<int16_t>::max()) return Err(std::strong_ordering::greater);
        if(ret < std::numeric_limits<int16_t>::min()) return Err(std::strong_ordering::less);
        return Ok(Self{round_cast<int16_t>(ret)});
    }

    constexpr void fill_bytes(const std::span<uint8_t, 2> bytes) const {
        bytes[0] = static_cast<uint8_t>(bits);
        bytes[1] = static_cast<uint8_t>(bits >> 8);
    }
};



struct [[nodiscard]] X2Code final{
    using Self = X2Code;
    int16_t bits;

    static constexpr Result<Self, std::strong_ordering> try_from_speed(const Angular<iq16> speed){
        const auto ret = speed.to_turns() << 8;
        if(ret > std::numeric_limits<int16_t>::max()) return Err(std::strong_ordering::greater);
        if(ret < std::numeric_limits<int16_t>::min()) return Err(std::strong_ordering::less);
        return Ok(Self{math::round_cast<int16_t>(ret)});
    }

    constexpr void fill_bytes(const std::span<uint8_t, 2> bytes) const {
        bytes[0] = static_cast<uint8_t>(bits);
        bytes[1] = static_cast<uint8_t>(bits >> 8);
    }
};



struct [[nodiscard]] X1Code final{
    using Self = X1Code;
    int32_t bits;

    static constexpr Result<Self, std::strong_ordering> try_from_turns(const iq15 turns){
        const auto ret = turns.to_bits();
        return Ok(Self{math::round_cast<int32_t>(ret)});
    }

    constexpr void fill_bytes(const std::span<uint8_t, 4> bytes) const {
        bytes[0] = static_cast<uint8_t>(bits);
        bytes[1] = static_cast<uint8_t>(bits >> 8);
        bytes[2] = static_cast<uint8_t>(bits >> 16);
        bytes[3] = static_cast<uint8_t>(bits >> 24);
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

    constexpr void fill_bytes(const std::span<uint8_t, 2> bytes) const {
        bytes[0] = static_cast<uint8_t>(bits);
        bytes[1] = static_cast<uint8_t>(bits >> 8);
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

    constexpr void fill_bytes(const std::span<uint8_t, 2> bytes) const {
        bytes[0] = static_cast<uint8_t>(bits);
        bytes[1] = static_cast<uint8_t>(bits >> 8);
    }
};


struct [[nodiscard]] TxHeader final{
    static constexpr void fill_bytes(const std::span<uint8_t, 2> bytes) {
        bytes[0] = 0xfe;
        bytes[1] = 0xee;
    }
};

struct [[nodiscard]] TxContext final{
    ModeInfo mode_settings;
    TorqueCode torque_code;
    X2Code x2_code;
    X1Code x1_code;
    KpCode kp_code;
    KdCode kd_code;

    constexpr void fill_bytes(const std::span<uint8_t, 13> bytes) const{
        auto & self = *this;
        self.mode_settings.fill_bytes(bytes.subspan<2-2,1>());
        self.torque_code.fill_bytes(bytes.subspan<3-2,2>());
        self.x2_code.fill_bytes(bytes.subspan<5-2,2>());
        self.x1_code.fill_bytes(bytes.subspan<9-2,4>());
        self.kp_code.fill_bytes(bytes.subspan<7-2,2>());
        self.kd_code.fill_bytes(bytes.subspan<13-2,2>());
    }
    
    template<typename Serializer>
    void sink_to(Serializer && serializer) const {
        auto & self = *this;
        std::array<uint8_t, 17> buffer;
        TxHeader::fill_bytes(std::span(buffer).template subspan<0, 2>());
        self.fill_bytes(std::span(buffer).template subspan<2, 13>());
        const auto crc_code = encrypt::crc::crc16_ccitt(std::span(buffer).template subspan<0, 15>());
        buffer[15] = static_cast<uint8_t>(crc_code & 0xff);
        buffer[16] = static_cast<uint8_t>(crc_code >> 8);

        serializer.sink_bytes(std::span(buffer));
    } 
};


struct [[nodiscard]] RxHeader final{
    static constexpr void fill_bytes(const std::span<uint8_t, 2> bytes) {
        bytes[0] = 0xfd;
        bytes[1] = 0xee;
    }
};


struct [[nodiscard]] TempCode final{
    uint8_t bits;

    using Self = TempCode;

    constexpr int8_t to_celeius() const {
        return std::bit_cast<uint8_t>(bits);
    }

    constexpr Self from_bits(uint8_t b) const {
        return Self{.bits = std::bit_cast<uint8_t>(b)};
    }

    constexpr void fill_bytes(std::span<uint8_t, 1> bytes) const{
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

    [[nodiscard]] constexpr bool is_ok() const {
        return bits == 0;
    }

    [[nodiscard]] constexpr bool is_err() const {return not is_ok();}

    [[nodiscard]] constexpr bool is_invalid() const {return bits > 4;}

    [[nodiscard]] constexpr Kind unwrap() const {
        if(is_ok()) [[unlikely]] __builtin_trap();
        return static_cast<Kind>(bits);
    }
};


struct [[nodiscard]] RxContext final{
    using Self = RxContext;


    ModeInfo mode_info;
    TorqueCode torque_code;
    X2Code x2_code;
    X1Code x1_code;
    TempCode temp_code;

    struct [[nodiscard]] Misc final{
        uint16_t err_bits : 3;
        uint16_t force_bits : 12;
        uint16_t __resv__:1;

        static constexpr Misc from_bytes(std::span<const uint8_t, 2> bytes){
            const uint16_t ret = bytes[0] | (bytes[1] << 8);
            return std::bit_cast<Misc>(ret);
        }

        constexpr void fill_bytes(std::span<uint8_t, 2> bytes) const {
            const auto b16 = std::bit_cast<uint16_t>(*this);
            bytes[0] = static_cast<uint8_t>(b16);
            bytes[1] = static_cast<uint8_t>(b16 >> 8);
        }
    };

    Misc misc;

    static constexpr RxContext from_bytes(std::span<const uint8_t, 12> bytes) {
        Self self;
        self.mode_info.bits = bytes[0];
        self.torque_code.bits = static_cast<int16_t>(bytes[1] | (bytes[2] << 8));
        self.x2_code.bits = bytes[3] | (bytes[4] << 8);
        self.x1_code.bits = bytes[5] | (bytes[6] << 8) | (bytes[7] << 16) | (bytes[8] << 24);
        self.temp_code.bits = bytes[9];
        self.misc = Misc::from_bytes(bytes.subspan<10, 2>());
        return self;
    }

    constexpr void fill_bytes(const std::span<uint8_t, 12> bytes) const{
        auto & self = *this;
        self.mode_info.fill_bytes(bytes.subspan<0,1>());
        self.torque_code.fill_bytes(bytes.subspan<1,2>());
        self.x2_code.fill_bytes(bytes.subspan<3,2>());
        self.x1_code.fill_bytes(bytes.subspan<5,4>());
        self.temp_code.fill_bytes(bytes.subspan<9,1>());
        self.misc.fill_bytes(bytes.subspan<10,2>());
    }

    template<typename Serializer>
    Result<void, typename Serializer::Error> serialize(Serializer && serializer) const {
        auto & self = *this;
        std::array<uint8_t, 16> buffer;
        TxHeader::fill_bytes(std::span(buffer).template subspan<0, 2>());
        self.fill_bytes(std::span(buffer).template subspan<2, 12>());
        const auto crc_code = encrypt::crc::crc16_ccitt(std::span(buffer).template subspan<0, 14>());
        buffer[14] = static_cast<uint8_t>(crc_code & 0xff);
        buffer[15] = static_cast<uint8_t>(crc_code >> 8);

        return serializer.push_bytes(std::span(buffer));
    } 
};
}