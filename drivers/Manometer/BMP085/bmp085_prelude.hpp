#pragma once


#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"



// https://github.com/adafruit/Adafruit-BMP085-Library/blob/master/BMP085.cpp

namespace ymd::drivers{

struct BMP085_Prelude{
static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x77); // BMP085 I2C address

enum class Mode:uint8_t{
    UltraLowPower = 0,
    Standard = 1,
    HighRes = 2,
    UltraHighRes = 3
};

static constexpr uint8_t BMP085_CAL_AC1 = 0xAA;    // R   Calibration data (16 bits)
static constexpr uint8_t BMP085_CAL_AC2 = 0xAC;    // R   Calibration data (16 bits)
static constexpr uint8_t BMP085_CAL_AC3 = 0xAE;    // R   Calibration data (16 bits)
static constexpr uint8_t BMP085_CAL_AC4 = 0xB0;    // R   Calibration data (16 bits)
static constexpr uint8_t BMP085_CAL_AC5 = 0xB2;    // R   Calibration data (16 bits)
static constexpr uint8_t BMP085_CAL_AC6 = 0xB4;    // R   Calibration data (16 bits)
static constexpr uint8_t BMP085_CAL_B1 = 0xB6;     // R   Calibration data (16 bits)
static constexpr uint8_t BMP085_CAL_B2 = 0xB8;     // R   Calibration data (16 bits)
static constexpr uint8_t BMP085_CAL_MB = 0xBA;     // R   Calibration data (16 bits)
static constexpr uint8_t BMP085_CAL_MC = 0xBC;     // R   Calibration data (16 bits)
static constexpr uint8_t BMP085_CAL_MD = 0xBE;     // R   Calibration data (16 bits)

static constexpr uint8_t BMP085_CONTROL = 0xF4;         // Control register
static constexpr uint8_t BMP085_TEMPDATA = 0xF6;        // Temperature data register
static constexpr uint8_t BMP085_PRESSUREDATA = 0xF6;    // Pressure data register
static constexpr uint8_t BMP085_READTEMPCMD = 0x2E;     // Read temperature control register value
static constexpr uint8_t BMP085_READPRESSURECMD = 0x34; // Read pressure control register value


struct Coeffs{
    int16_t ac1, ac2, ac3, b1, b2, mb, mc, md;
    uint16_t ac4, ac5, ac6;

    static constexpr Coeffs from_default(){
        Coeffs ret;
        ret.ac6 = 23153;
        ret.ac5 = 32757;
        ret.mc = -8711;
        ret.md = 2868;
        ret.b1 = 6190;
        ret.b2 = 4;
        ret.ac3 = -14383;
        ret.ac2 = -72;
        ret.ac1 = 408;
        ret.ac4 = 32741;
        return ret;
    }

    constexpr int32_t computeB5(int32_t UT) const {
        int32_t X1 = (UT - (int32_t)ac6) * ((int32_t)ac5) >> 15;
        int32_t X2 = ((int32_t)mc << 11) / (X1 + (int32_t)md);
        return X1 + X2;
    }

    struct SeaLevelPresure{
        int32_t sealevel_pressure;

        constexpr float to_altitude(const float pressure) const{
            return 44330 * (1.0 - std::pow(pressure / sealevel_pressure, 0.1903));
        }
    };

    constexpr auto to_sea_level_pressure_converter(float altitude_meters) const {
        return [=](const int32_t pressure) -> SeaLevelPresure{
            return {(int32_t)(pressure / std::pow(1.0 - altitude_meters / 44330, 5.255))};
        };
    }

    constexpr float requalify_temperature(const int32_t UT) const{
        float temp;

        const auto B5 = computeB5(UT);
        temp = (B5 + 8) >> 4;
        temp /= 10;

        return temp;
    }

    constexpr int32_t requalify_pressure(const int32_t UT, const int32_t UP, const Mode mode) const{
        int32_t B3, B5, B6, X1, X2, X3, p;
        uint32_t B4, B7;

        B5 = computeB5(UT);

        #if BMP085_DEBUG_EN == 1
        DEBUG_PRINT("X1 = ", X1);
        DEBUG_PRINT("X2 = ", X2);
        DEBUG_PRINT("B5 = ", B5);
        #endif

        // do pressure calcs
        B6 = B5 - 4000;
        X1 = ((int32_t)b2 * ((B6 * B6) >> 12)) >> 11;
        X2 = ((int32_t)ac2 * B6) >> 11;
        X3 = X1 + X2;
        B3 = ((((int32_t)ac1 * 4 + X3) << std::bit_cast<uint8_t>(mode)) + 2) / 4;

        #if BMP085_DEBUG_EN == 1
        DEBUG_PRINT("B6 = ", B6);
        DEBUG_PRINT("X1 = ", X1);
        DEBUG_PRINT("X2 = ", X2);
        DEBUG_PRINT("B3 = ", B3);
        #endif

        X1 = ((int32_t)ac3 * B6) >> 13;
        X2 = ((int32_t)b1 * ((B6 * B6) >> 12)) >> 16;
        X3 = ((X1 + X2) + 2) >> 2;
        B4 = ((uint32_t)ac4 * (uint32_t)(X3 + 32768)) >> 15;
        B7 = ((uint32_t)UP - B3) * (uint32_t)(50000UL >> std::bit_cast<uint8_t>(mode));

        #if BMP085_DEBUG_EN == 1
        DEBUG_PRINT("X1 = ", X1);
        DEBUG_PRINT("X2 = ", X2);
        DEBUG_PRINT("B4 = ", B4);
        DEBUG_PRINT("B7 = ", B7);
        #endif

        if (B7 < 0x80000000) {
            p = (B7 * 2) / B4;
        } else {
            p = (B7 / B4) * 2;
        }
        X1 = (p >> 8) * (p >> 8);
        X1 = (X1 * 3038) >> 16;
        X2 = (-7357 * p) >> 16;

        #if BMP085_DEBUG_EN == 1
        DEBUG_PRINT("p = ", p);
        DEBUG_PRINT("X1 = ", X1);
        DEBUG_PRINT("X2 = ", X2);
        #endif

        p = p + ((X1 + X2 + (int32_t)3791) >> 4);
        #if BMP085_DEBUG_EN == 1
        DEBUG_PRINT("p = ", p);
        #endif
        return p;
    }


    friend OutputStream & operator<<(OutputStream & os, const Coeffs & self){ 
        os.println("ac1 = ", std::hex, self.ac1);
        os.println("ac2 = ", std::hex, self.ac2);
        os.println("ac3 = ", std::hex, self.ac3);
        os.println("ac4 = ", std::hex, self.ac4);
        os.println("ac5 = ", std::hex, self.ac5);
        os.println("ac6 = ", std::hex, self.ac6);

        os.println("b1 = ", std::hex, self.b1);
        os.println("b2 = ", std::hex, self.b2);

        os.println("mb = ", std::hex, self.mb);
        os.println("mc = ", std::hex, self.mc);
        os.println("md = ", std::hex, self.md);
        return os;
    }
};

enum class Error_Kind:uint8_t{
    InvalidId
};

DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)
template<typename T = void>
using IResult = Result<T, Error>;
};


}