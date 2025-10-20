#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/angle.hpp"

#include "hal/bus/spi/spidrv.hpp"


#include "drivers/Encoder/MagEncoder.hpp"


namespace ymd::drivers{

struct MA730_Prelude{
    using Error = EncoderError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class Width:uint8_t{
        W90, W180, W270, W360
    };

    enum class Phase:uint8_t{
        P0, P90, P180, P270
    };

    enum class MagThreshold:uint8_t{
        mT23, mT38, mT53, mT67, mT81, mT95, mT109, mT123
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

struct MA730_Regs:public MA730_Prelude{

    struct R8_ZeroDataLow:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::ZeroDataLow;
        uint8_t data;
    };

    struct R8_ZeroDataHigh:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::ZeroDataHigh;
        uint8_t data;
    };

    struct R8_Trim:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::Trim;
        uint8_t trim;
    };
    struct R8_TrimConfig:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::TrimConfig;
        uint8_t enableX:1;
        uint8_t enableY:1;
        uint8_t :6;
    };

    struct R8_ZParameters:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::ZParameters;
        uint8_t :2;
        Phase zPhase :2;
        Width zWidth :2;
        uint8_t ppt:2;
    };

    struct R8_PulsePerTurn:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::PulsePerTurn;
        uint8_t data;
    };

    struct R8_Threshold:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::Threshold;
        uint8_t :2;
        uint8_t thresholdHigh :3;
        uint8_t thresholdLow :3;
    };

    struct R8_Direction:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::Direction;
        uint8_t :7;
        uint8_t direction :1;
    };

    struct R8_Magnitude:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::Magnitude;
        uint8_t :2;
        uint8_t mgl1:1;
        uint8_t mgl2:1;
        uint8_t :2;
        uint8_t magnitudeLow :1;
        uint8_t magnitudeHigh :1;
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