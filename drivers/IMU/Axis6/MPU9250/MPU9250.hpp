#pragma once

#include "drivers/IMU/Axis6/MPU6050/mpu6050.hpp"
#include "drivers/IMU/Magnetometer/AK8963/AK8963.hpp"

namespace ymd::drivers{


class MPU9250{
public:
    enum class Error{

    };

    // [[nodiscard]] virtual Result<void, Error> write_reg(const uint8_t addr, const uint8_t data);
    [[nodiscard]] Result<void, Error> write_reg(const uint8_t addr, const uint8_t data);

    // [[nodiscard]] virtual Result<void, Error> read_reg(const uint8_t addr, uint8_t & data);
    [[nodiscard]] Result<void, Error> read_reg(const uint8_t addr, uint8_t & data);

    // [[nodiscard]] virtual Result<void, Error> read_burst(const uint8_t reg_addr, int16_t * datas, const size_t len);
    [[nodiscard]] Result<void, Error> read_burst(const uint8_t reg_addr, int16_t * datas, const size_t len);
private:

public:
    Result<void, Error> init();
    Result<void, Error> verify();
    
    Result<void, Error> enableChainMode(Enable en = EN);
};


}