#pragma once

#include "primitive/arithmetic/fraction.hpp"

#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/conn/i2c/i2cdrv.hpp"


// https://wiki.lckfb.com/zh-hans/lspi/module/sensor/aht10-temp-humi-sensor.html

namespace ymd::drivers{
static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x38);

enum class Mode:uint8_t{
    Nor = 0b00,
    Cyc = 0b01,
    Cmd = 0b10
};

static constexpr uint8_t MEASURE_COMMAND = 0xac;

struct AHT10_Prelude{

// 比特位	意义	描述
// Bit[7]	忙闲指示	1=设备忙，处于测量模式 0=设备闲，处于休眠模式
// Bit[6~5]	读取工作模式	00 当前处于 NOR 模式 01 当前处于 CYC 模式 11 当前处于 CMD 模式
// Bit[4]	保留	保留
// Bit[3]	校准使能位	1=已校准 0=未校准
// Bit[2~0]	保留	保留
struct [[nodiscard]] Status final{
    uint8_t __resv__:3;
    uint8_t cali_en:1;
    uint8_t __resv2__:1;
    Mode mode:2;
    uint8_t is_busy:1;
};

struct [[nodiscard]] HumidityCode final{
    uint32_t bits;

    constexpr float into() const noexcept {
        return bits / 1048576.0 * 100.0;
    }
};

struct [[nodiscard]] TemperatureCode final{
    uint32_t bits;

    constexpr float into() const noexcept {
        return (bits/1048576.0) * 200 - 50;
    }
};


struct [[nodiscard]] Packet final{
    std::array<uint8_t, 6> bytes;

    constexpr Status status() const noexcept {
        return std::bit_cast<Status>(bytes[0]);
    }

    constexpr HumidityCode humidity_code() const noexcept {
        const uint32_t bits = (((bytes[1]<<12) | (bytes[2]<<4)) | (bytes[3]>>4));
        return HumidityCode{bits};
    }

    constexpr TemperatureCode temperature_code() const noexcept {
        const uint32_t bits = ((bytes[3] &0x0F) << 16 ) | ( bytes[4] << 8) | bytes[5];
        return TemperatureCode{bits};
    }
};
};


}