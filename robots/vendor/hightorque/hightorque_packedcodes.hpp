#pragma once

#include "hightorque_primitive.hpp"

namespace ymd::robots::hightorque{

static constexpr int16_t NIL_BITS = 0x8000;
static constexpr int16_t clamp_bits(int32_t bits){
    constexpr int32_t MAX_BITS = (int32_t)std::numeric_limits<uint16_t>::max();
    constexpr int32_t MIN_BITS = NIL_BITS + 1;
    bits = std::min(bits, MAX_BITS);
    bits = std::max(bits, MIN_BITS);

    return static_cast<int16_t>(bits);
}

struct [[nodiscard]] PositionCode final{
    using Self = PositionCode;

    static constexpr ElementType ELEMENT_TYPE = ElementType::B2;

    // 单位 0.0001 圈，如 pos = 5000 表示转到 0.5 圈的位置。
    static constexpr uint32_t LSB_PER_TURN = 10000;
    static constexpr double MAX_TURNS = std::numeric_limits<int16_t>::max() / double(LSB_PER_TURN);
    static constexpr double MIN_TURNS = std::numeric_limits<int16_t>::min() / double(LSB_PER_TURN);


    int16_t bits;

    static constexpr Self nil(){return Self{NIL_BITS};}
    static constexpr Self zero(){return Self{0};}

    [[nodiscard]] constexpr bool is_nil() const {return bits == NIL_BITS;}
    
    static constexpr Option<Self> try_from_turns(const iq16 turns){
        constexpr iq16 MAX_TURNS_T = static_cast<iq16>(MAX_TURNS);
        constexpr iq16 MIN_TURNS_T = static_cast<iq16>(MIN_TURNS);
        // return Self{static_cast<int16_t>(turns * 10000)};
        if(turns >= MAX_TURNS_T) return None;
        else if(turns <= MIN_TURNS_T) return None;
        const int32_t bits = static_cast<int32_t>(static_cast<int64_t>(turns.to_bits()) * 10000) >> turns.NUM_Q;
        return Some(Self{clamp_bits(bits)});
    }

    static constexpr Option<Self> try_from_angle(const Angular<iq16> angle){
        return try_from_turns(angle.to_turns());
    }

    constexpr Angular<iq16> to_angle() const noexcept {
        constexpr auto FACTOR = uq32(1.0 / 10000);
        const iq16 turns = iq16(bits) * FACTOR;
        return Angular<iq16>::from_turns(turns);
    }
};

struct [[nodiscard]] SpeedCode{
    using Self = SpeedCode;

    // 速度：单位 0.00025 转/秒，如 val = 1000 表示 0.25 转/秒
    static constexpr ElementType ELEMENT_TYPE = ElementType::B2;

    int16_t bits;

    static constexpr Self nil(){return Self{NIL_BITS};}
    static constexpr Self zero(){return Self{0};}

    [[nodiscard]] constexpr bool is_nil() const {return bits == NIL_BITS;}

    static constexpr Self from_angular_speed(const Angular<iq16> angular_speed){
        const int32_t bits = static_cast<int32_t>(angular_speed.to_turns() * 4000);
        return Self{clamp_bits(bits)};
    }

    constexpr Angular<iq16> to_angular_speed() const noexcept {
        constexpr auto FACTOR = uq32(1.0 / 4000);
        const iq16 turns = iq16(bits) * FACTOR;
        return Angular<iq16>::from_turns(turns);
    }
};

static_assert(sizeof(PositionCode) == 2);


struct [[nodiscard]] AccelerationCode{
    // 加速度：单位 0.01 转/秒^2，如 acc = 40 表示 0.4 转/秒^2
    using Self = AccelerationCode;

    static constexpr ElementType ELEMENT_TYPE = ElementType::B2;

    int16_t bits;

    static constexpr Self nil(){return Self{NIL_BITS};}
    static constexpr Self zero(){return Self{0};}

    [[nodiscard]] constexpr bool is_nil() const {return bits == NIL_BITS;}

    static constexpr Self from_angular_acceleration(const Angular<iq16> angular_acceleration){
        const int32_t bits = static_cast<int32_t>(angular_acceleration.to_turns() * 100);
        return Self{clamp_bits(bits)};
    }

    constexpr Angular<iq16> to_angular_acceleration() const noexcept {
        constexpr auto FACTOR = uq32(1.0 / 100);
        const iq16 turns = iq16(bits) * FACTOR;
        return Angular<iq16>::from_turns(turns);
    }
};

struct [[nodiscard]] TorqueCode{
    // 单位：0.01 NM，如 torque = 110 表示最大力矩为 1.1NM
    using Self = TorqueCode;

    static constexpr ElementType ELEMENT_TYPE = ElementType::B2;


    int16_t bits;

    static constexpr Self nil(){return Self{NIL_BITS};}
    static constexpr Self zero(){return Self{0};}

    [[nodiscard]] constexpr bool is_nil() const {return bits == NIL_BITS;}

    static constexpr Self from_nm(const iq16 torque){
        const int32_t bits = static_cast<int32_t>(torque * 100);
        return Self{clamp_bits(bits)};
    }
    constexpr iq16 to_nm() const noexcept {
        constexpr auto FACTOR = uq32(1.0 / 100);
        return iq16(bits) * FACTOR;
    }
};

static_assert(sizeof(AccelerationCode) == 2);

struct [[nodiscard]] PhaseVoltageCode{
    // 电压：单位 0.1 V，如 val = 10 1 V
    using Self = PhaseVoltageCode;

    static constexpr ElementType ELEMENT_TYPE = ElementType::B2;


    int16_t bits;


    static constexpr Self nil(){return Self{NIL_BITS};}
    static constexpr Self zero(){return Self{0};}

    [[nodiscard]] constexpr bool is_nil() const {return bits == NIL_BITS;}

    static constexpr Self from_volts(const iq16 volts){
        const int32_t bits = static_cast<int32_t>(volts * 10);
        return Self{clamp_bits(bits)};
    }

    constexpr iq16 to_volts() const noexcept {
        constexpr auto FACTOR = uq32(1.0 / 10);
        return iq16(bits) * FACTOR;
    }
};

struct [[nodiscard]] CurrentCode{
    // 电流：单位 0.1 A，如 10 => 1 A
    using Self = CurrentCode;

    static constexpr ElementType ELEMENT_TYPE = ElementType::B2;

    int16_t bits;

    static constexpr Self nil(){return Self{NIL_BITS};}
    static constexpr Self zero(){return Self{0};}

    [[nodiscard]] constexpr bool is_nil() const {return bits == NIL_BITS;}

    static constexpr Self from_amps(const iq16 current){
        const int32_t bits = static_cast<int32_t>(current * 10);
        return Self{clamp_bits(bits)};
    }

    constexpr iq16 to_amps() const noexcept {
        constexpr auto FACTOR = uq32(1.0 / 10);
        return iq16(bits) * FACTOR;
    }
};


}