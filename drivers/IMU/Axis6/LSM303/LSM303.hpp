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

    IResult<> init(deviceType device = device_auto, sa0State sa0 = sa0_auto);

    IResult<> enable_default();


    IResult<> read_acc();
    IResult<> read_mag();
    IResult<> read();
private:
    details::LSM303_Regs regs_ = {};

    Option<math::Vec3<int16_t>> m_max = None; // maximum magnetometer values, used for calibration
    Option<math::Vec3<int16_t>> m_min = None; // minimum magnetometer values, used for calibration

    deviceType _device = device_auto;; // chip type (D, DLHC, DLM, or DLH)

    uint8_t acc_address;
    uint8_t mag_address;

    static const int dummy_reg_count = 6;
    regAddr translated_regs[dummy_reg_count + 1]; // index 0 not used

    IResult<> test_reg(const uint8_t reg_addr, const uint8_t reg_val);
    
    IResult<> write_acc_reg(uint8_t reg_addr, uint8_t reg_val);
    // uint8_t readAccReg(uint8_t reg_addr);
    IResult<> write_mag_reg(uint8_t reg_addr, uint8_t reg_val);
    // uint8_t readMagReg(int reg_addr);

    IResult<> write_reg(uint8_t reg_addr, uint8_t reg_val);
    IResult<> read_reg(uint8_t reg_addr, uint8_t & reg_val);
};

}