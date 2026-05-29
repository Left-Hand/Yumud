#pragma once


#include <tuple>

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "core/math/real.hpp"

#include "hal/conn/i2c/i2cdrv.hpp"

// https://blog.csdn.net/weixin_46510976/article/details/149939581
// https://github.com/flightErmao/07-gg-all-inone/blob/d0ffc86df0ac4e25c4066bc7c2384896cdda8b5a/driverframework/L2_device/04_BARO/01_SPL06_001/inc/spl06001.hpp

namespace ymd::drivers::spl06001{


static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x76);


enum class [[nodiscard]] RegAddr:uint8_t{
    CHIP_ID = 0x0D,
    RESET = 0x0C,
    INT_STATUS = 0x0A,
    FIFO_STATUS = 0x0B,
    PRS_CFG = 0x06,
    TMP_CFG = 0x07,
    CTRL_MEAS = 0x08,
    CONFIG = 0x09,

    // Calibration data
    CALIB_START = 0x10
};

enum class [[nodiscard]] SampleRate:uint8_t{
    // Samplerate
    _1  = 0x00,
    _2  = 0x01,
    _4  = 0x02,
    _8  = 0x03,
    _16  = 0x04,
    _32  = 0x05,
    _64  = 0x06,
    _128  = 0x07,
};;


enum class [[nodiscard]] Oversampling:uint8_t{
    _1X     = 0x00,
    _2X     = 0x01,
    _4X     = 0x02,
    _8X     = 0x03,
    _16X        = 0x04,
    _32X        = 0x05,
    _64X        = 0x06,
    _128X       = 0x07,
};

enum class [[nodiscard]] WorkMode:uint8_t{
    Sleep = 0x00,
    Normal = 0x07
};

static constexpr size_t SPL06_REG_CALIB_LEN = 18;


static constexpr int32_t parse_nested_3bytes(std::span<const uint8_t, 3> bytes){
    int32_t x = 0;
    x = (int32_t)bytes[0] << 16 | (int32_t)bytes[1] << 8 | (int32_t)bytes[2];
    x = (x & 0x800000) ? (0xFF000000 | x) : x;
    return x;
};


// The Pressure Data registers contains the 24 bit (3 bytes) 2's complement pressure measurement 
// value. If the FIFO is enabled, the register will contain the FIFO pressure and/or temperature results 
// (please see FIFO Operation). Otherwise, the register contains the pressure measurement results 
// and will not be cleared after read.
struct [[nodiscard]] alignas(4) RawPair final{
    int32_t ut;
    int32_t up;

    constexpr void init_from_bytes(std::span<const uint8_t, 6> bytes){
        up = parse_nested_3bytes(std::span<const uint8_t, 3>{bytes.data(), 3});
        ut = parse_nested_3bytes(std::span<const uint8_t, 3>{bytes.data() + 3, 3});
    }
};

/*! @brief Calibration parameters structure for SPL06 sensor */
struct [[nodiscard]] alignas(4) CalibParam final {
    int16_t c0;  /**< Temperature coefficient c0 */
    int16_t c1;  /**< Temperature coefficient c1 */
    int32_t c00; /**< Pressure coefficient c00 */
    int32_t c10; /**< Pressure coefficient c10 */
    int16_t c01; /**< Pressure coefficient c01 */
    int16_t c11; /**< Pressure coefficient c11 */
    int16_t c20; /**< Pressure coefficient c20 */
    int16_t c21; /**< Pressure coefficient c21 */
    int16_t c30; /**< Pressure coefficient c30 */

    constexpr void set_coeffs_from_bytes(std::span<const uint8_t, 18> bytes){
        auto & self = *this;

        // Parse c0 (registers 0x10-0x11)
        self.c0 = (int16_t)bytes[0] << 4 | bytes[1] >> 4;
        if (self.c0 & 0x0800) self.c0 = (int16_t)(0xF000 | self.c0);

        // Parse c1 (registers 0x11-0x12)
        self.c1 = (int16_t)(bytes[1] & 0x0F) << 8 | bytes[2];
        if (self.c1 & 0x0800) self.c1 = (int16_t)(0xF000 | self.c1);

        // Parse c00 (registers 0x13-0x15)
        self.c00 = (int32_t)bytes[3] << 12 | (int32_t)bytes[4] << 4 | (int32_t)bytes[5] >> 4;
        if (self.c00 & 0x080000) self.c00 = (int32_t)(0xFFF00000 | self.c00);

        // Parse c10 (registers 0x15-0x17)
        self.c10 = (int32_t)(bytes[5] & 0x0F) << 16 | (int32_t)bytes[6] << 8 | bytes[7];
        if (self.c10 & 0x080000) self.c10 = (int32_t)(0xFFF00000 | self.c10);

        // Parse c01 (registers 0x18-0x19)
        self.c01 = (int16_t)bytes[8] << 8 | bytes[9];

        // Parse c11 (registers 0x1A-0x1B)
        self.c11 = (int16_t)bytes[10] << 8 | bytes[11];

        // Parse c20 (registers 0x1C-0x1D)
        self.c20 = (int16_t)bytes[12] << 8 | bytes[13];

        // Parse c21 (registers 0x1E-0x1F)
        self.c21 = (int16_t)bytes[14] << 8 | bytes[15];

        // Parse c30 (registers 0x20-0x21)
        self.c30 = (int16_t)bytes[16] << 8 | bytes[17];
    } 

    #if 0
    template<typename T>
    [[nodiscard]] constexpr T compensate_pressure(const int32_t ut, const int32_t up) const {
        auto & self = *this;
        T fTsc = (T)ut / (T)kT_;
        T fPsc = (T)up / (T)kP_;
        T qua2 = self.c10 + fPsc * (self.c20 + fPsc * self.c30);
        T qua3 = fTsc * fPsc * (self.c11 + fPsc * self.c21);
        T pressure = self.c00 + fPsc * qua2 + fTsc * self.c01 + qua3;
        return pressure;
    }

    #endif

    #if 0
    [[nodiscard]] constexpr int32_t compensate_pressure_i32(const int32_t ut, const int32_t up) const {
        auto & self = *this;
        int64_t left = ((int64_t)ut << 16);
        int32_t fTsc = (int32_t)(left / (int64_t)kT_);
        left = ((int64_t)up << 16);
        int32_t fPsc = (int32_t)(left / (int64_t)kP_);
        int32_t qua2 = ((int32_t)self.c20 << 8) + ((int32_t)((int32_t)self.c30 * fPsc) >> 8);
        int32_t qua3 = ((int32_t)self.c10 << 8) + (int32_t)(((int64_t)qua2 * (int64_t)fPsc) >> 16);
        int32_t pressure = ((int32_t)self.c00 << 8) + (int32_t)(((int64_t)qua3 * (int64_t)fPsc) >> 16);
        pressure += ((int32_t)((int32_t)self.c01 * fTsc) >> 8);
        qua2 = ((int32_t)self.c11 << 8) + ((int32_t)((int32_t)self.c21 * fPsc) >> 8);
        qua3 = (int32_t)(((int64_t)qua2 * (int64_t)fPsc) >> 16);
        pressure += (int32_t)(((int64_t)qua3 * (int64_t)fTsc) >> 16);
        pressure = (pressure >> 8);
        return pressure;
    }

    [[nodiscard]] constexpr int32_t compensate_temperature_i32(int32_t ut) {
        auto & self = *this;

        int64_t left = ((int64_t)ut << 16);
        int32_t fTsc = (int32_t)(left / (int64_t)kT_);
        int32_t temperature = ((int32_t)self.c0 << 7) + ((int32_t)((int32_t)self.c1 * fTsc) >> 8);
        temperature = (temperature * 100) / 256;
        return temperature;
    }

    #endif


};

}