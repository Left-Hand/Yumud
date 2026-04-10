#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "primitive/arithmetic/angular.hpp"

#include "hal/conn/spi/spidrv.hpp"
#include "drivers/encoder/encoder.hpp"


namespace ymd::drivers{

struct MA782_Prelude{
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

    enum class [[nodiscard]] RegAddr:uint8_t{
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

    using Command = uint16_t;

    enum class [[nodiscard]] CommandType:uint8_t{
        ReadAngle = 0b000,
        ReadReg = 0b010,
        WriteReg = 0b100,
        StoreSingleToNvm = 0b111,
        StoreAllToNvm = 0b110,
        RestoreAllFromNvm = 0b101,
        ClearError = 0b001
    };

    struct [[nodiscard]] CommandFactory final{
        static constexpr Command read_reg(const uint8_t reg_addr){
            uint16_t bits = 0;
            bits |= (static_cast<uint16_t>(CommandType::ReadReg) << 13);
            bits |= (static_cast<uint16_t>(reg_addr) << 8);
            return bits;
        }

        static constexpr Command write_reg(const uint8_t reg_addr, const uint8_t reg_val){
            return with_dual_args(CommandType::WriteReg, reg_addr, reg_val);
        }

        static constexpr Command pragram_reg(const uint8_t reg_addr, const uint8_t reg_val){
            return with_dual_args(CommandType::StoreSingleToNvm, reg_addr, reg_val);
        }
    private:
        static constexpr Command with_dual_args(const CommandType type, const uint8_t arg1, const uint8_t arg2){
            uint16_t bits = 0;
            bits |= (static_cast<uint16_t>(type) << 13);
            bits |= (static_cast<uint16_t>(arg1) << 8);
            bits |= (static_cast<uint16_t>(arg2));
            return bits;
        }
    }
};

struct MA782_Regset:public MA782_Prelude{
    //0x00
    struct R8_ZeroLow:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr::ZeroDataLow;
        uint8_t bits;
    };

    //0x01
    struct R8_ZeroHigh:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr::ZeroDataHigh;
        uint8_t bits;
    };

    //0x02
    struct R8_Trim:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr::Trim;
        uint8_t trim;
    };

    //0x03
    struct R8_TrimConfig:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr::TrimConfig;
        uint8_t enable_x:1;
        uint8_t enable_y:1;
        uint8_t :6;
    };

    //0x06
    struct R8_Threshold:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr::Threshold;
        uint8_t :2;
        MagThreshold high :3;
        MagThreshold low :3;
    };

    struct R8_Direction:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr::Direction;
        uint8_t :7;
        uint8_t direction_is_ccw :1;
    };

    struct R8_Magnitude:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr::Magnitude;
        uint8_t :2;
        uint8_t mgl1:1;
        uint8_t mgl2:1;
        uint8_t :2;
        uint8_t magnitude_low :1;
        uint8_t magnitude_high :1;
    };

    R8_ZeroLow zero_low_reg = {};
    R8_ZeroHigh zero_high_reg = {};
    R8_Trim trim_reg = {};

    R8_TrimConfig trim_config_reg = {};
    R8_Threshold threshold_reg = {};
    R8_Direction direction_reg = {};
    R8_Magnitude magnitude_reg = {};

    static_assert(sizeof(zero_low_reg) == 1);
    static_assert(sizeof(zero_high_reg) == 1);
    static_assert(sizeof(trim_reg) == 1);
    static_assert(sizeof(trim_config_reg) == 1);
    static_assert(sizeof(threshold_reg) == 1);
    static_assert(sizeof(direction_reg) == 1);
    static_assert(sizeof(magnitude_reg) == 1);
};

};