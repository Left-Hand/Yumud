#pragma once

// https://blog.csdn.net/qq_43547520/article/details/131880984


#include "core/io/regs.hpp"
#include "core/utils/Errno.hpp"
#include "core/utils/Result.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

struct MCP4725_Collections{
    static constexpr auto DEFAULT_I2C_ADDR = 
        hal::I2cSlaveAddr<7>::from_u8(0b1100'001);
    static constexpr size_t VREF_5V = 5000;

    enum class Error_Kind{

    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)
};

class MCP4725 final:public MCP4725_Collections{
public:
    MCP4725(hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){}    

    //使用快速模式写命令写DAC寄存器
    Result<void, Error> MCP4725_WriteData_Voltage(uint16_t Vout)   //电压单位mV
    {
        const uint8_t Dn = ( 4096 * Vout) / VREF_5V; //这里的VREF_5V宏定义为5000
        const uint8_t temp = (0x0F00 & Dn) >> 8;  //12位数据。0000XXXX XXXXXXXX 

        if(const auto res = i2c_drv_.write_reg(temp, Dn);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
    
    Result<void, Error> MCP4725_WriteData_Digital(uint16_t data)   //12位数字量
    {
        if(const auto res = i2c_drv_.write_reg(uint8_t(data >> 8), uint8_t(data));
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
private:
    hal::I2cDrv i2c_drv_;
};

}