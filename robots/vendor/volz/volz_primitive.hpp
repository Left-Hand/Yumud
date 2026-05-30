#pragma once

#include <cstdint>


namespace ymd::robots::volz{


enum class [[nodiscard]] Command : uint8_t {
    SetExtendedPosition = 0xDC,
    ExtendedPositionResponse = 0x2C,
    ReadCurrent = 0xB0,
    CurrentResponse = 0x30,
    ReadVoltage = 0xB1,
    VoltageResponse = 0x31,
    ReadTemperature = 0xC0,
    TemperatureResponse = 0x10,
};

}