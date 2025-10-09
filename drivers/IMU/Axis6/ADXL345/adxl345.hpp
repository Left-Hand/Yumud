#pragma once

#include "adxl345_prelude.hpp"

namespace ymd::drivers{

class ADXL345:
    public AccelerometerIntf, 
    public GyroscopeIntf,
    public ADXL345_Prelude
{

public:
    explicit ADXL345(const hal::I2cDrv & i2c_drv): 
        phy_(i2c_drv){;}

    explicit ADXL345(hal::I2cDrv && i2c_drv): 
        phy_(i2c_drv){;}

    explicit ADXL345(
        Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        phy_(hal::I2cDrv(i2c, addr)){;}

    explicit ADXL345(const hal::SpiDrv & _spi_drv): 
        phy_(_spi_drv){;}

    explicit ADXL345(hal::SpiDrv && _spi_drv): 
        phy_(_spi_drv){;}

    explicit ADXL345(Some<hal::Spi *> spi, const hal::SpiSlaveIndex index): 
        phy_(hal::SpiDrv(spi, index)){;}

    // uint8_t get_device_id(){
    //     read_reg(RegAddress::DeviceID, deviceIDReg);
    //     return deviceIDReg.data;
    // }

    [[nodiscard]] IResult<Vec3<q24>> read_acc();
    [[nodiscard]] IResult<Vec3<q24>> read_gyr();
private:
    AnalogDeviceIMU_Phy phy_;

    [[nodiscard]] IResult<> write_reg(const RegAddress reg_address, const uint8_t reg_data);

    [[nodiscard]] IResult<> read_reg(const RegAddress reg_address, uint8_t & reg_data);
};

};