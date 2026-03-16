#pragma once

#include "core/io/regs.hpp"

#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/math/real.hpp"

#include "hal/conn/i2c/i2cdrv.hpp"

namespace ymd::drivers{

struct INA219_Prelude{

    static constexpr auto DEFAULT_I2C_ADDR = 
        hal::I2cSlaveAddr<7>::from_u7(0x80 >> 1);

    enum class Error_Kind:uint8_t{

    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class AverageTimes:uint8_t{
        _1 = 0,
        _4 = 1,
        _16 = 2,
        _64 = 3,
        _128 = 4,
        _256 = 5,
        _512 = 6,
        _1024 = 7
    };

    enum class ConversionTime:uint8_t{
        _140us = 0, _204us, _332us, _588us, _1_1ms, _2_116_ms, _4_156ms, _8_244ms
    };

    enum class HalfScale:uint8_t{
        // _1= 0,
        // _1By2= 1,
        // _1By4= 2,
        // _1By4= 3

        _40mV = 0,
        _80mV = 0,
        _160mV = 0,
        _320mV = 0,
    };


    using RegAddr = uint8_t ;


    struct [[nodiscard]] BusVoltageCode final{
        //lsb is 4mv
        uint16_t bits;

        constexpr uint32_t to_mv() const{
            return bits * 4;
        }

        constexpr uq16 to_volts() const{
            constexpr size_t SHIFTS = 25;
            constexpr uint32_t RATIO = (1u << SHIFTS) * 4 / 1000;
            static_assert(RATIO < (1u << (16 + 3)));
            return uq16::from_bits((static_cast<uint32_t>(bits) * RATIO) >> (SHIFTS - 16));
        }
    };


    struct [[nodiscard]] ShuntVoltCode final{
        //-320mv ~ 320mv max
        //10uv lsb
        int16_t bits;

        [[nodiscard]] constexpr int32_t to_uv() const{
            return int32_t(bits) * 10;
        }

        [[nodiscard]] constexpr int32_t to_mv() const{
            return int32_t(bits) / 100;
        }

        [[nodiscard]] constexpr iq31 to_volts() const{
            constexpr uint32_t RATIO = (1ull << 48) * (10 * 1E-6);
            return iq31::from_bits((static_cast<int64_t>(bits) * RATIO) >> 17);
        }

        [[nodiscard]] static constexpr ShuntVoltCode from_uv(const int32_t uv) {
            return ShuntVoltCode(uv / 10);
        }

        [[nodiscard]] static constexpr ShuntVoltCode from_mv(const int32_t mv) {
            return ShuntVoltCode(mv * 100);
        }

        [[nodiscard]] static constexpr ShuntVoltCode from_volts(const iq31 volts) {
            constexpr uint64_t RATIO = static_cast<uint64_t>(1e5) << 1;
            return ShuntVoltCode((volts.to_bits() * RATIO) >> 32); 
        }
    };

    struct [[nodiscard]] PowerCode final{
        uint16_t bits;
    };


};


struct INA219_Regs:public INA219_Prelude{
    struct [[nodiscard]] R16_Config:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x00};

        uint16_t mode:3;
        uint16_t sdac:4;
        uint16_t bdac:4;
        uint16_t pg:2;
        uint16_t brng:1;
        uint16_t __resv__:1;
        uint16_t rst:1;
    }DEF_R16(config_reg)



    struct [[nodiscard]] R16_ShuntVolt:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x01};
        uint16_t bits;
    }DEF_R16(shunt_volt_reg)

    struct [[nodiscard]] R16_BusVolt:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x02};
        uint16_t overflow:1;
        uint16_t conversion_ready:1;
        uint16_t :1;
        uint16_t bd:13;

        constexpr BusVoltageCode volt_code() const{
            return BusVoltageCode{bd};
        }
    }DEF_R16(bus_volt_reg)

    struct [[nodiscard]] R16_Power:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x03};
        PowerCode code;
    }DEF_R16(power_reg)


    
    struct [[nodiscard]] R16_Current:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x04};
        // CurrentCode code;
        uint16_t bits;
    }DEF_R16(current_reg)
    
    struct [[nodiscard]] R16_Calibration:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x05};
        int16_t bits;
    }DEF_R16(calibration_reg)
};
}
