#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/math/real.hpp"

#include "hal/conn/i2c/i2cdrv.hpp"

namespace ymd::drivers{

struct INA226_Prelude{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x80 >> 1);
    // ASCII 的 TI。
    static constexpr uint16_t VALID_MANU_ID = 0x5449;

    // INA226
    static constexpr uint16_t VALID_CHIP_ID = 0x2260;

    enum class Error_Kind{
        ChipIdVerifyFailed,
        ManuIdVerifyFailed,
        ResTooBig
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class AverageTimes:uint16_t{
        _1 = 0,
        _4 = 1,
        _16 = 2,
        _64 = 3,
        _128 = 4,
        _256 = 5,
        _512 = 6,
        _1024 = 7
    };

    enum class ConversionTime:uint16_t{
        _140us = 0, _204us, _332us, _588us, _1_1ms, _2_116_ms, _4_156ms, _8_244ms
    };

    using RegAddr = uint8_t;

    static constexpr iq16 VOLTAGE_LSB_MV = iq16(1.25);

    [[nodiscard]] static constexpr uint16_t mv_to_sv_code(const int32_t mv){
        uint16_t bits = uint16_t(mv * (10000 / 25));
        return bits;
    }

    [[nodiscard]] static constexpr uint16_t volts_to_sv_code(const iq16 volts){
        #if 0
        constexpr uint32_t RATIO = (10000'000 / 25);
        uint16_t bits = uint16_t((static_cast<int64_t>(volts.to_bits()) * RATIO) >> 16);
        return bits;
        #else
        return mv_to_sv_code(round_cast<int32_t>(volts * 1000));
        #endif
    }

    [[nodiscard]] static constexpr uint16_t uv_to_sv_code(const int32_t uv){
        constexpr uint32_t UQ32_2_BY_5 = static_cast<uint32_t>((1ull << 32) * 2 / 5); 
        uint16_t bits = uint16_t((int64_t(uv) * UQ32_2_BY_5) >> 32);
        return bits;
    }


    [[nodiscard]] static constexpr iq16 sv_code_to_volts(const uint16_t sv_code){ 
        constexpr uint32_t RATIO = static_cast<uint32_t>((1ull << 48) * 2.5 / 1000000);
        return iq16::from_bits((static_cast<int64_t>(std::bit_cast<int16_t>(sv_code)) * RATIO) >> 32);
    }

    [[nodiscard]] static constexpr int32_t sv_code_to_mv(const uint16_t sv_code){ 
        constexpr uint32_t RATIO = static_cast<uint32_t>((1ull << 32) * 2.5 / 1000);
        return int32_t((static_cast<int64_t>(std::bit_cast<int16_t>(sv_code)) * RATIO) >> 32);
    }

    [[nodiscard]] static constexpr int32_t sv_code_to_uv(const uint16_t sv_code){ 
        int32_t uv = 0;
        uv += (std::bit_cast<int16_t>(sv_code) >> 1);
        uv += (std::bit_cast<int16_t>(sv_code) << 1);
        return int32_t(uv);
    }

    [[nodiscard]] static constexpr iq16 bv_code_to_volts(uint16_t bv_code){ 
        bv_code &= 0x7FFF;
        constexpr uint64_t RATIO = static_cast<uint64_t>((1ull << 48) * (40.96 / 0x7fff));
        return iq16::from_bits((uint64_t(bv_code) * RATIO) >> 32);
    }

    [[nodiscard]] static constexpr uint32_t bv_code_to_mv(uint16_t bv_code){ 
        bv_code &= 0x7FFF;

        #if 0
        // mv * 1.25
        uint32_t mv = 0;
        mv += ((bv_code) >> 2);
        mv += ((bv_code) << 0);
        return uint32_t(mv);
        #else
        constexpr uint64_t RATIO = static_cast<uint64_t>((1ull << 32) * (40.96 / 0x7fff) * 1000);
        uint32_t mv = (uint64_t(bv_code) * RATIO) >> 32;
        return uint32_t(mv);
        #endif
    }

    [[nodiscard]] static constexpr uint32_t bv_code_to_uv(uint16_t bv_code){ 
        bv_code &= 0x7FFF;

        constexpr uint64_t RATIO = static_cast<uint64_t>((1ull << 32) * (40.96 / 0x7fff) * 1000000);
        uint32_t uv = (uint64_t(bv_code) * RATIO) >> 32;
        return uint32_t(uv);
    }


    struct [[nodiscard]] ShuntVoltageCode final{
        using Self = ShuntVoltageCode;
        uint16_t bits;

        constexpr int32_t to_uv() const noexcept {
            return sv_code_to_uv(bits);
        }

        constexpr int32_t to_mv() const noexcept {
            return sv_code_to_mv(bits);
        }

        constexpr iq16 to_volts() const noexcept {
            return sv_code_to_volts(bits);
        }
    };


    struct [[nodiscard]] BusVoltageCode final{
        using Self = BusVoltageCode;
        uint16_t bits;

        constexpr int32_t to_uv() const noexcept {
            return bv_code_to_uv(bits);
        }

        constexpr int32_t to_mv() const noexcept {
            return bv_code_to_mv(bits);
        }

        constexpr iq16 to_volts() const noexcept {
            return bv_code_to_volts(bits);
        }
    };

    static constexpr AverageTimes times_to_avtimes(const uint16_t times){
        const uint8_t temp = __builtin_ctz(times);

        if(times <= 64){
            return std::bit_cast<AverageTimes>(uint16_t(temp / 2));
        }else{
            return std::bit_cast<AverageTimes>(uint16_t(4 + (temp - 7))); 
        }
    } 
};

struct INA226_Regs:public INA226_Prelude{
    struct R16_Config:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x00};

        uint16_t shunt_voltage_enable :1;
        uint16_t bus_voltage_enable :1;
        uint16_t continuos :1;
        ConversionTime shunt_voltage_conversion_time:3;
        ConversionTime bus_voltage_conversion_time:3;
        AverageTimes average_times:3;
        uint16_t __resv__:3;
        uint16_t rst:1;
    }DEF_R16(config_reg)

    struct R16_ShuntVolt:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x01};
        ShuntVoltageCode code;
    }DEF_R16(shunt_volt_reg)

    struct R16_BusVolt:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x02};
        BusVoltageCode code;
    }DEF_R16(bus_volt_reg)

    struct R16_Power:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x03};
        int16_t bits;
    }DEF_R16(power_reg)

    struct R16_Current:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x04};
        int16_t bits;
    }DEF_R16(current_reg)
    
    struct R16_Calibration:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x05};
        int16_t bits;
    }DEF_R16(calibration_reg)
    
    struct R16_Mask:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x06};

        uint16_t alert_latch_enable:1;
        uint16_t alert_polarity:1;
        uint16_t math_overflow:1;
        uint16_t conv_ready_flag:1;
        uint16_t alert_flag:1;
        uint16_t __resv__:5;
        uint16_t conv_ready:1;
        uint16_t power_overlimit:1;
        uint16_t bus_under_voltage:1;
        uint16_t bus_over_voltage:1;
        uint16_t shunt_under_voltage:1;
        uint16_t shunt_over_voltage:1;
    }DEF_R16(mask_reg)

    struct R16_AlertLimit:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x07};
        uint16_t bits;
    }DEF_R16(alert_limit_reg)

    struct R16_Manufacture:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0xfe};
        uint16_t bits;
    }DEF_R16(manufacture_reg)

    struct R16_ChipId:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0xff};
        uint16_t bits;
    }DEF_R16(chip_id_reg)
};



}
