#pragma once

#include "drivers/device_defs.h"
#include "drivers/IMU/IMU.hpp"


namespace ymd::drivers{

class IST8310:public Magnetometer{
public:
    scexpr uint8_t default_i2c_addr =  0x0E << 1;


    enum class AverageTimes:uint8_t{
        _1 = 0b000,
        _2 = 0b001,
        _4 = 0b010,
        _8 = 0b011,
        _16 = 0b100,
    };
    
    IST8310(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    IST8310(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    IST8310(hal::I2c & i2c, const uint8_t addr = default_i2c_addr):i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    void init();
    void update();

    bool verify();

    void reset();

    void enableContious(const bool en = true);

    void setXAverageTimes(const AverageTimes times);
    void setYAverageTimes(const AverageTimes times);

    int getTemperature();

    bool busy();
    void enableInterrupt(const bool en = true);
    void setInterruptLevel(const BoolLevel lv);
    bool getInterruptStatus();

    void sleep(const bool en = true);

    Option<Vector3> getMagnet() override;

protected:
    using RegAddress = uint8_t;


    struct WhoAmIReg:public Reg8<>{
        scexpr RegAddress address = 0x00;
        scexpr uint8_t expected_value = 0x10;
        uint8_t data;
    };

    struct Status1Reg:public Reg8<>{
        scexpr RegAddress address = 0x02;

        uint8_t drdy:1;
        uint8_t ovf:1;
        uint8_t :6;
    };

    struct AxisXReg:public Reg16i<>{
        scexpr RegAddress address = 0x03;

        int16_t data;
    };

    struct AxisYReg:public Reg16i<>{
        scexpr RegAddress address = 0x05;

        int16_t data;
    };

    struct AxisZReg:public Reg16i<>{
        scexpr RegAddress address = 0x07;

        int16_t data;
    };

    struct Status2Reg:public Reg8<>{
        scexpr RegAddress address = 0x09;

        uint8_t :3;
        uint8_t on_int:1;
        uint8_t :4;
    };

    struct Ctrl1Reg:public Reg8<>{
        scexpr RegAddress address = 0x0A;

        uint8_t awake:1;
        uint8_t cont:1;
        uint8_t :6;
    };

    struct Ctrl2Reg:public Reg8<>{
        scexpr RegAddress address = 0x0B;

        uint8_t reset:1;
        uint8_t :1;
        uint8_t drdy_level:1;
        uint8_t int_en:1;
        uint8_t :4;
    };

    struct SelfTestReg:public Reg8<>{
        scexpr RegAddress address = 0x0C;

        uint8_t :6;
        uint8_t st_en:1;
        uint8_t :1;
    };

    struct TempReg:public Reg8<>{
        scexpr RegAddress address = 0x1C;
        uint16_t data;

        operator int() const {
            return ((uint8_t(*this) * int(0.8 * 65536) >> 16) - 75);
        }
    };


    struct AverageReg:public Reg8<>{
        scexpr RegAddress address = 0x41;

        uint8_t x_times:3;
        uint8_t y_times:3;
        uint8_t :2;
    };



    hal::I2cDrv i2c_drv_;

    WhoAmIReg whoami_reg;
    Status1Reg status1_reg;
    AxisXReg axis_x_reg;
    AxisYReg axis_y_reg;
    AxisZReg axis_z_reg;
    Status2Reg status2_reg;
    Ctrl1Reg ctrl1_reg;
    Ctrl2Reg ctrl2_reg;
    SelfTestReg selftest_reg;
    TempReg temp_reg; 
    AverageReg average_reg;
    void writeReg(const RegAddress address, const uint8_t reg){
        i2c_drv_.writeReg(uint8_t(address), reg);
    }

    void readReg(const RegAddress address, uint8_t & reg){
        i2c_drv_.readReg(uint8_t(address), reg);
    }

    void requestPool(const RegAddress addr, int16_t * data, const size_t len){
        i2c_drv_.readMulti(uint8_t(addr), data, len, LSB);
    }

    void setAverageTimes(bool is_x, AverageTimes times);
};

}