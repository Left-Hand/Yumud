#pragma once

#include "core/io/regs.hpp"
#include "drivers/IMU/IMU.hpp"
#include "drivers/IMU/details/AsahiKaseiIMU.hpp"


namespace ymd::drivers{


class AK8975_Prelude{
public:
    using Error = ImuError;

    enum class Mode:uint8_t{
        PowerDown = 0b0000,
        SingleMeasurement = 0b0001,
        SelfTest = 0b1000,
        FuseRomAccess = 0b1111,
    };


    using RegAddr = uint8_t;

    template<typename T = void>
    using IResult = Result<T, Error>;

    static constexpr uint8_t CORRECT_CHIP_ID = 0x48;
    static constexpr  auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x68 >> 1);
};


}