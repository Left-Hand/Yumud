#pragma once

#include "core/io/regs.hpp"
#include "core/math/real.hpp"
#include "core/utils/result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

struct BMP280_Prelude{
enum class [[nodiscard]] Mode:uint8_t{
    Sleep, Single, Cont = 0x03
};

enum class [[nodiscard]] TempratureSampleMode:uint8_t{
    SKIP, 
    _16Bits, 
    _17Bits, 
    _18Bits, 
    _19Bits, 
    _20Bits = 0x07
};

enum class [[nodiscard]] PressureSampleMode:uint8_t{
    SKIP, 
    _16Bits, 
    _17Bits, 
    _18Bits, 
    _19Bits, 
    _20Bits = 0x07
};

enum class [[nodiscard]] DataRate:uint8_t{
    _200Hz, 
    _16Hz, 
    _8Hz, 
    _4Hz, 
    _2Hz, 
    _1Hz, 
    _0_5Hz, 
    _0_25Hz
};

enum class [[nodiscard]] FilterCoefficient:uint8_t{
    OFF, 
    _2, 
    _4, 
    _8, 
    _16
};

static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0xec >> 1);
static constexpr uint8_t VALID_CHIPID = 0x58;
static constexpr uint8_t RESET_KEY = 0xB6;

using RegAddr = uint8_t;

enum class [[nodiscard]] Error_Kind{
    WrongChipId,
    NoPressure
};

DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

template<typename T = void>
using IResult = Result<T, Error>;

struct Coeffs{
    uint16_t digT1;
    int16_t digT2;
    int16_t digT3;
    uint16_t digP1;
    int16_t digP2;
    int16_t digP3;
    int16_t digP4;
    int16_t digP5;
    int16_t digP6;
    int16_t digP7;
    int16_t digP8;
    int16_t digP9;

    constexpr uint32_t convert(const uint32_t adc_T, const uint32_t adc_P){
        #if 1
        uint32_t var1 = (((float)adc_T)/16384.0f-((float)digT1)/1024.0f)*((float)digT2);
        uint32_t var2 = ((((float)adc_T)/131072.0f-((float)digT1)/8192.0f)*(((float)adc_T)
                    /131072.0f-((float)digT1)/8192.0f))*((float)digT3);

        uint32_t t_fine = (unsigned long)(var1+var2);

        var1 = ((float)t_fine/2.0f)-64000.0f;
        var2 = var1*var1*((float)digP6)/32768.0f;
        var2 = var2 +var1*((float)digP5)*2.0f;
        var2 = (var2/4.0f)+(((float)digP4)*65536.0f);
        var1 = (((float)digP3)*var1*var1/524288.0f+((float)digP2)*var1)/524288.0f;
        var1 = (1.0f+var1/32768.0f)*((float)digP1);
        uint32_t p = 1048576.0f-(float)adc_P;
        p = (p-(var2/4096.0f))*6250.0f/var1;
        var1 = ((float)digP9)*p*p/2147483648.0f;
        var2 = p*((float)digP8)/32768.0f;
        return (p+(var1+var2+((float)digP7))/16.0f);
        #else
        uint32_t var1 = (((float)adc_T)/16384-((float)digT1)/1024)*((float)digT2);
        uint32_t var2 = ((((float)adc_T)/131072-((float)digT1)/8192)*(((float)adc_T)
                    /131072-((float)digT1)/8192))*((float)digT3);

        uint32_t t_fine = uint32_t(var1+var2);

        var1 = (t_fine >> 1) - 64000;
        var2 = var1*var1*(digP6)/32768;
        var2 = var2 +var1*(digP5)*2;
        var2 = (var2/4)+((digP4)*65536);
        var1 = ((digP3)*var1*var1/524288+(digP2)*var1)/524288;
        var1 = (1+var1/32768)*(digP1);
        uint32_t p = 1048576-adc_P;
        p = (p-(var2/4096))*6250/var1;
        var1 = (digP9)*p*p/2147483648;
        var2 = p*(digP8)/32768;
        return (p+(var1+var2+(digP7))/16);
        #endif
    }
};
};

struct BMP280_Regset final: public BMP280_Prelude {

    struct R8_ChipID:public Reg8<>{
        static constexpr RegAddr ADDRESS=0xD0;
        uint8_t bits;
    }DEF_R8(chipid_reg)

    struct R8_Reset:public Reg8<>{
        static constexpr RegAddr ADDRESS=0xE0;

        using Reg8<>::operator=;
        uint8_t bits;
    }DEF_R8(reset_reg)

    struct R8_Status:public Reg8<>{
        static constexpr RegAddr ADDRESS=0xF3;
        uint8_t im:1;
        uint8_t __resv1__:2;
        uint8_t busy:1;

        uint8_t __resv2__:4;
    }DEF_R8(status_reg)

    struct R8_Ctrl:public Reg8<>{
        static constexpr RegAddr ADDRESS=0xF4;
        Mode mode:2;
        PressureSampleMode   osrs_p:3;
        TempratureSampleMode osrs_t:3;
    }DEF_R8(ctrl_reg)

    struct R8_Config:public Reg8<>{
        static constexpr RegAddr ADDRESS=0xF5;
        uint8_t spi3_en:1;
        uint8_t __resv__:1;
        FilterCoefficient filter_coeff:3;
        DataRate t_sb:3;
    }DEF_R8(config_reg)

    struct R16_Pressure:public Reg16<>{
        static constexpr RegAddr ADDRESS=0xF6;
        uint16_t bits;
    }DEF_R16(pressure_reg)

    struct R8_PressureX:public Reg8<>{
        static constexpr RegAddr ADDRESS=0xF9;
        uint8_t bits;
    }DEF_R8(pressure_x_reg)

    struct R16_Temperature:public Reg16<>{
        static constexpr RegAddr ADDRESS=0xFA;
        uint16_t bits;
    }DEF_R16(temperature_reg)

    struct R8_TemperatureX:public Reg8<>{
        static constexpr RegAddr ADDRESS=0xFC;
        uint8_t bits;
    }DEF_R8(temperature_x_reg)

};

}