#pragma once

#include <cstdint>
#include <span>

namespace ymd::canopen::primitive{

// 同步报文是不带数据的，但是紧急报文是带8个字节的数据的，8个字节拉满

// 紧急报文的8个字节数据被分为了3个部分 
// 应急错误代码[0-1]
// 错误寄存器[2]
// 自定义错误区域[3-7]

enum class [[nodiscard]] EmcyError:uint16_t{
    None = 0x0000,
    Generic = 0x1000,
    Current = 0x2000,
    InputCurrent = 0x2100,
    InternalCurrent = 0x2200,
    OutputCurrent = 0x2300,
    Voltage = 0x3000,
    BusbarVoltage = 0x3100,
    InternalVoltage = 0x3200,
    OutputVoltage = 0x3300,
    Temperature = 0x4000,
    SituationTemperature = 0x4100,
    InternalTemperature = 0x4200,
    Hardware = 0x5000,

    Software = 0x6000,
    InternalSoftware = 0x6100,
    UserSoftware = 0x6200,
    SetterSoftware = 0x6300,
    AdjuntDevice = 0x7000,
    Monitor = 0x8000,
    Communication = 0x8100,
    CommunicationOverload = 0x8100,
    Passive = 0x8120,
    NodeProtect = 0x8130,
    BusRecovery = 0x8140,
    Protocol = 0x8200,
    UnhandledPdo = 0x8210,
    OutOfRange = 0x8220,
    External = 0x9000,
    AddictiveFunction = 0xf000,
    Specified = 0xf100,

};

struct [[nodiscard]] EmcyErrorCode{
    EmcyError err;

    inline std::span<const uint8_t, 2> as_bytes() const {
        return std::span<const uint8_t, 2>(reinterpret_cast<const uint8_t*>(this), 2);
    }

    inline void fill_bytes(std::span<uint8_t, 2> pbuf) const {
        pbuf[0] = as_bytes()[0];
        pbuf[1] = as_bytes()[1];
    }
};

static_assert(sizeof(EmcyErrorCode) == 2);

}