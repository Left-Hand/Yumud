#pragma once

//15位磁编码器

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "primitive/arithmetic/angular.hpp"

#include "hal/conn/spi/spidrv.hpp"

#include "drivers/encoder/encoder.hpp"


namespace ymd::drivers{


struct MT6826S_Prelude { 
using Error = EncoderError;

template<typename T = void>
using IResult = Result<T, Error>;

// @brief MT6826S commands
enum class [[nodiscard]] Command:uint16_t{
    ReadReg = 0b0011,
    WriteReg = 0b0110,
    BurnReg = 0b1100,
    ReadAngle = 0b1010,
};

enum class [[nodiscard]] RegAddr:uint16_t{
    UserId = 0x001,
    AngleHigh = 0x003,
    AngleLow = 0x004,
    Status = 0x005,
};


enum class [[nodiscard]] Hysteresis:uint8_t{
    LSB1, LSB2, LSB4, LSB8,
    LSB0, LSB0_25, LSB00_5
};

enum class [[nodiscard]] ZeroPulseWidth:uint8_t{
    LSB1, LSB2, LSB4, LSB8,LSB12, LSB16, HALF
};

enum class [[nodiscard]] PwmFreq:uint8_t{
    _994Hz,_497Hz
};

struct [[nodiscard]] alignas(1) Status final{
    uint8_t overspeed:1;
    uint8_t magweak:1;
    uint8_t uvlo:1;

    [[nodiscard]] bool any_fault() const noexcept {
        return std::bit_cast<uint8_t>(*this);
    }

    [[nodiscard]] EncoderFaultBitFields to_encoder_fault() const noexcept {
        EncoderFaultBitFields fault = EncoderFaultBitFields::zero();
        if(overspeed){
            fault.is_over_speed = true;
        }
        
        if(magweak){
            fault.mag_strength = EncoderFaultBitFields::MagStrength::Low;
        } 
        
        if(uvlo){
            fault.supply_voltage_level = EncoderFaultBitFields::SupplyVoltageLevel::Under;
        }
        return fault;
    }
};


struct [[nodiscard]] alignas(4) Packet final{
    uint16_t angle_msb_u15;
    Status status;
    uint8_t crc;

    [[nodiscard]] std::span<uint8_t, 4> as_bytes_mut() {
        return std::span<uint8_t, 4>(reinterpret_cast<uint8_t *>(this), 4);
    }

    [[nodiscard]] constexpr bool is_crc_valid() const noexcept {
        //TODO
        return true;
    }
};
static_assert(sizeof(Packet) == 4);

};


}