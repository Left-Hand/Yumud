#pragma once

#include "primitive/can/bxcan_frame.hpp"
#include "core/utils/Option.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/bits/bitfield_proxy.hpp"
#include "core/math/real.hpp"


#include "primitive/arithmetic/PerUnit.hpp"


namespace ymd::robots::cybergear{

struct [[nodiscard]] StatusBitFields final{
    using Self = StatusBitFields;
    uint8_t can_id;
    uint8_t under_voltage:1;
    uint8_t over_current:1;
    uint8_t over_temp:1;
    uint8_t mag_enc_err:1;

    uint8_t hall_enc_err:1;
    uint8_t uncalibrated:1;
    uint8_t mode:2;

    [[nodiscard]] constexpr bool is_running() const noexcept {return mode == 2;}
    [[nodiscard]] constexpr bool is_reset() const noexcept {return mode == 0;}
    [[nodiscard]] constexpr bool is_calibrating() const noexcept {return mode == 1;}
};

static_assert(sizeof(StatusBitFields) == 2);


enum class [[nodiscard]] Error:uint8_t{
    PRAGRAM_UNHANDLED,
    PRAGRAM_TODO,
    RET_DLC_SHORTER,
    RET_DLC_LONGER,
    RET_UNKOWN_CMD,
    INPUT_OUT_OF_RANGE,
    INPUT_LOWER_THAN_LIMIT,
    INPUT_HIGHER_THAN_LIMIT,
    MOTOR_NOT_INITED,
};

enum class [[nodiscard]] Command:uint8_t{
    GET_DEVICE_ID = 0,
    SEND_CTRL1 = 1,
    FBK_CTRL1 = 2,
    EN_MOT = 3,
    DISEN_MOT = 4,
    SET_MACHINE_HOME = 6,
    SET_CAN_ID = 7,
    READ_PARA = 17,
    WRITE_PARA = 18,
    FBK_FAULT = 21,
};


template<typename T = void>
using IResult = Result<T, Error>;

struct [[nodiscard]] TemperatureCode final{
    uint16_t bits;

    constexpr iq16 to_celsius() const noexcept {
        return iq16(bits) / 10;
    }
};

DEF_PER_UNIT(RadCode, uint16_t, -2 * (2 * M_PI), 2 * (2 * M_PI))
DEF_PER_UNIT(OmegaCode, uint16_t, -30 * (2 * M_PI), 30 * (2 * M_PI))
DEF_PER_UNIT(TorqueCode, uint16_t, -12, 12)
DEF_PER_UNIT(KpCode, uint16_t, 0, 500)
DEF_PER_UNIT(KdCode, uint16_t, 0, 5)


struct alignas(4) [[nodiscard]] CgId final{

    uint32_t bits;


    template <typename Self>
    [[nodiscard]] constexpr auto cmd(this Self && self) {
        return make_bitfield_proxy<24, 29, cybergear::Command>(&self.bits);}

    template <typename Self>
    [[nodiscard]] constexpr auto high(this Self && self) {
        return make_bitfield_proxy<8, 24, uint16_t>(&self.bits);}

    template <typename Self>
    [[nodiscard]] constexpr auto fault(this Self && self) {
        return make_bitfield_proxy<8, 24, uint16_t>(&self.bits);}

    template <typename Self>
    [[nodiscard]] constexpr auto low(this Self && self) {
        return make_bitfield_proxy<0, 8, uint8_t>(&self.bits);}

    [[nodiscard]] static constexpr 
    CgId from_parts(const cybergear::Command cmd, const uint16_t high, const uint8_t low) {
        CgId self;

        self.cmd().set(cmd);
        self.high().set(high);
        self.low().set(low);

        return self;
    }

    [[nodiscard]] static constexpr CgId from_bits(const uint32_t bits) {
        CgId self;
        self.bits = bits;
        return self;
    }

    [[nodiscard]] constexpr uint32_t to_bits() const noexcept {return bits;}

    [[nodiscard]] constexpr hal::CanExtId to_extid() const noexcept {return hal::CanExtId::from_bits(bits);}
};


struct alignas(4)  [[nodiscard]] TxContext final{
    uint64_t bits;

    template <typename Self>
    [[nodiscard]] constexpr auto cmd_rad(this Self && self) {
        return make_bitfield_proxy<0, 16, RadCode>(&self.bits);}

    template <typename Self>
    [[nodiscard]] constexpr auto cmd_omega(this Self && self) {
        return make_bitfield_proxy<16, 32, OmegaCode>(&self.bits);}

    template <typename Self>
    [[nodiscard]] constexpr auto cmd_kp(this Self && self) {
        return make_bitfield_proxy<32, 48, KpCode>(&self.bits);}

    template <typename Self>
    [[nodiscard]] constexpr auto cmd_kd(this Self && self) {
        return make_bitfield_proxy<48, 64, KdCode>(&self.bits);}

    constexpr hal::ClassicCanPayload to_can_payload() const noexcept {return hal::ClassicCanPayload::from_u64(bits);}
};

static_assert(sizeof(TxContext) == 8);  

struct alignas(4)  [[nodiscard]] RxContext final{
    uint64_t bits;

    template <typename Self>
    [[nodiscard]] constexpr auto radians(this Self && self) {            
        return make_bitfield_proxy<0, 16, RadCode>(&self.bits);}

    template <typename Self>
    [[nodiscard]] constexpr auto omega(this Self && self) {          
        return make_bitfield_proxy<16, 32, OmegaCode>(&self.bits);}

    template <typename Self>
    [[nodiscard]] constexpr auto torque(this Self && self) {         
        return make_bitfield_proxy<32, 48, TorqueCode>(&self.bits);}

    template <typename Self>
    [[nodiscard]] constexpr auto temperature(this Self && self) {    
        return make_bitfield_proxy<48, 64, TemperatureCode>(&self.bits);}
};

static_assert(sizeof(RxContext) == 8);


static constexpr Err<Error> make_err_from_cmp(const std::weak_ordering ord){
    if (ord == std::weak_ordering::less) {
        return Err<Error>(Error::INPUT_LOWER_THAN_LIMIT);
    } else if (ord == std::weak_ordering::greater) {
        return Err<Error>(Error::INPUT_HIGHER_THAN_LIMIT);
    } else {
        __builtin_unreachable();
    }
}

}