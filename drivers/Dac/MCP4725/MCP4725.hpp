#pragma once

// https://blog.csdn.net/qq_43547520/article/details/131880984


#include "mcp4725_prelude.hpp"
namespace ymd::drivers{

class MCP4725 final:public MCP4725_Prelude{
public:
    explicit MCP4725(hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){}    
    explicit MCP4725(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){}    

    //使用快速模式写命令写DAC寄存器
    [[nodsicard]] IResult<> write_data_volt(uint16_t Vout)   //电压单位mV
    {
        const uint8_t Dn = ( 4096 * Vout) / VREF_5V; //这里的VREF_5V定义为5000
        const uint8_t temp = (0x0F00 & Dn) >> 8;  //12位数据。0000XXXX XXXXXXXX 

        if(const auto res = i2c_drv_.write_reg(temp, Dn);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
    
    [[nodsicard]] IResult<> write_data_u16(uint16_t data)   //12位数字量
    {
        if(const auto res = i2c_drv_.write_reg(uint8_t(data >> 8), uint8_t(data));
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
private:
    hal::I2cDrv i2c_drv_;
};

}