#pragma once


#include "core/math/real.hpp"
#include "core/utils/Result.hpp"
#include "primitive/arithmetic/angular.hpp"
#include "core/utils/Errno.hpp"


#include "primitive/hal_result.hpp"


namespace ymd::drivers{

namespace details{
enum class [[nodiscard]] EncoderError_Kind:uint8_t{
    UnknownPackage,
    EEpromProgramFailed,
    CantSetup,

    InvalidPc,
    InvalidPc2,
    InvalidCrc,

    RegProgramFailed,
    RegProgramResponseFormatInvalid,
    ValueOverflow,
    ValueUnderflow,
    InvalidRxFormat,

    SpiIsNotImplementedYet,
    I2cIsNotImplementedYet,

    Unreachable
};

// Magnetic,//磁编码器
// Spin,//旋变编码器
// LinearAnalog,//线性模拟信号编码器
// SincosAnalog,//sincos模拟信号编码器
// Pwm,//线性数字信号编码器
// Hall6//六步霍尔



struct alignas(4) [[nodiscard]] EncoderFaultBitFields final{
    using Self = EncoderFaultBitFields;
    enum class MagStrength:uint8_t{
        Proper = 0b00,
        Lost = 0b01,
        Low = 0b10,
        High = 0b11,
    };

    enum class SupplyVoltageLevel:uint8_t{
        Proper = 0b00,
        Lost = 0b01,
        Under = 0b10,
        Over = 0b11
    };

    union{
        struct{
            uint8_t arg0;
            uint8_t arg1;
            uint8_t arg2;
        };

        uint8_t args[3];
    };

    MagStrength mag_strength:2;
    SupplyVoltageLevel supply_voltage_level:2;

    uint8_t is_over_speed:1;
    uint8_t is_over_tempreature:1;

    [[nodiscard]] static constexpr Self zero(){
        return std::bit_cast<Self>(uint32_t(0));
    }

    [[nodiscard]] constexpr uint32_t to_u32() const {
        return std::bit_cast<uint32_t>(*this);
    }
    [[nodiscard]] constexpr bool is_ok(){
        return to_u32() & 0xff00'0000;
    }
};

static_assert(sizeof(EncoderFaultBitFields) == 4);

}
DEF_ERROR_SUMWITH_HALERROR(EncoderError, details::EncoderError_Kind)
}

namespace ymd{
OutputStream& operator << (OutputStream& os, const drivers::details::EncoderError_Kind value);
}


namespace ymd::drivers{

using details::EncoderFaultBitFields;



}

