#pragma once

//这个驱动还未实现

// MIT license
// https://github.com/pololu/lsm303-arduino/blob/master/LSM303.h

// CC 4.0 BY-SA版权
// https://blog.csdn.net/cp1300/article/details/86997168

#include "details/LSM303_Prelude.hpp"

namespace ymd::drivers{

class LSM303:public details::LSM303_Prelude{
public:

    LSM303(void);

    [[nodiscard]] IResult<> init(deviceType device = device_auto, sa0State sa0 = sa0_auto);

    [[nodiscard]] IResult<> enable_default();


    [[nodiscard]] IResult<> read_acc();
    [[nodiscard]] IResult<> read_mag();
    [[nodiscard]] IResult<> read();
private:
    details::LSM303_Regs regs_ = {};

    Option<Vec3<int16_t>> m_max = None; // maximum magnetometer values, used for calibration
    Option<Vec3<int16_t>> m_min = None; // minimum magnetometer values, used for calibration

    deviceType _device = device_auto;; // chip type (D, DLHC, DLM, or DLH)

    uint8_t acc_address;
    uint8_t mag_address;

    static const int dummy_reg_count = 6;
    regAddr translated_regs[dummy_reg_count + 1]; // index 0 not used

    [[nodiscard]] IResult<> test_reg(const uint8_t reg, const uint8_t val){return Ok();}
    
    [[nodiscard]] IResult<> write_acc_reg(uint8_t reg, uint8_t value);
    // uint8_t readAccReg(uint8_t reg);
    [[nodiscard]] IResult<> write_mag_reg(uint8_t reg, uint8_t value);
    // uint8_t readMagReg(int reg);

    [[nodiscard]] IResult<> write_reg(uint8_t reg, uint8_t value);
    [[nodiscard]] IResult<> read_reg(int reg, uint8_t & data);
};

}