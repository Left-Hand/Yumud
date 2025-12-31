#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "primitive/arithmetic/angular.hpp"

#include "hal/bus/spi/spidrv.hpp"


#include "drivers/Encoder/MagEncoder.hpp"


namespace ymd::drivers{

struct MA730_Prelude{
    using Error = EncoderError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class ZeroPulseWidth:uint8_t{
        _90deg, _180deg, _270deg, _360deg
    };

    enum class ZeroPulsePhase:uint8_t{
        _0deg, _90deg, _180deg, _270deg
    };

    enum class MagThreshold:uint8_t{
        _23mT, _38mT, _53mT, _67mT, _81mT, _95mT, _109mT, _123mT
    };

    enum class RegAddr:uint8_t{
        ZeroDataLow,
        ZeroDataHigh,
        Trim,
        TrimConfig,
        ZParameters,
        PulsePerTurn,
        Threshold,
        Direction = 9,
        Magnitude = 27
    };

};

struct MA730_Regset:public MA730_Prelude{

    struct R8_ZeroDataLow:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::ZeroDataLow;
        uint8_t bits;
    };

    struct R8_ZeroDataHigh:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::ZeroDataHigh;
        uint8_t bits;
    };

    struct R8_Trim:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::Trim;
        uint8_t trim;
    };

    struct R8_TrimConfig:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::TrimConfig;
        uint8_t enable_x:1;
        uint8_t enable_y:1;
        uint8_t :6;
    };

    struct R8_ZParameters:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::ZParameters;
        uint8_t :2;
        ZeroPulsePhase z_phase :2;
        ZeroPulseWidth z_width :2;
        uint8_t ppt:2;
    };

    struct R8_PulsePerTurn:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::PulsePerTurn;
        uint8_t bits;
    };

    struct R8_Threshold:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::Threshold;
        uint8_t :2;
        MagThreshold high :3;
        MagThreshold low :3;
    };

    struct R8_Direction:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::Direction;
        uint8_t :7;
        uint8_t direction_is_ccw :1;
    };

    struct R8_Magnitude:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::Magnitude;
        uint8_t :2;
        uint8_t mgl1:1;
        uint8_t mgl2:1;
        uint8_t :2;
        uint8_t magnitude_low :1;
        uint8_t magnitude_high :1;
    };

    R8_ZeroDataLow zero_data_low_reg = {};
    R8_ZeroDataHigh zero_data_high_reg = {};
    R8_Trim trim_reg = {};

    R8_TrimConfig trim_config_reg = {};
    R8_ZParameters z_parameters_reg = {};
    R8_PulsePerTurn pulse_per_turn_reg = {};
    R8_Threshold threshold_reg = {};
    R8_Direction direction_reg = {};
    R8_Magnitude magnitude_reg = {};
};

};