#pragma once

// Copyright 2023 IOsetting <iosetting(at)outlook.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#include "core/io/regs.hpp"

#include "drivers/IMU/IMU.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "core/utils/result.hpp"



namespace ymd::drivers{

class L3G4200D:public AccelerometerIntf{
public:
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0xD0);

    using Error = ImuError;
    
    template<typename T = void>
    using IResult = Result<T, Error>;


    enum class Dps{
        _2000DPS = 0b10,
        _500DPS  = 0b01,
        _250DPS  = 0b00
    };

    enum class OdrBw{
        _800_110_Hz  = 0b1111,
        _800_50_Hz   = 0b1110,
        _800_35_Hz   = 0b1101,
        _800_30_Hz   = 0b1100,
        _400_110_Hz  = 0b1011,
        _400_50_Hz   = 0b1010,
        _400_25_Hz   = 0b1001,
        _400_20_Hz   = 0b1000,
        _200_70_Hz   = 0b0111,
        _200_50_Hz   = 0b0110,
        _200_25_Hz   = 0b0101,
        _200_12_5_Hz = 0b0100,
        _100_25_Hz   = 0b0001,
        _100_12_5_Hz = 0b0000
    } ;
public:
    L3G4200D(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR)
        :i2c_drv_(hal::I2cDrv(i2c, addr)){}
    L3G4200D(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){}

    struct Config{
        Dps dps;
        OdrBw odrbw;
    };

    IResult<> init(const Config & cfg);
    IResult<> reconf(const Config & cfg);

    IResult<Dps> get_dps();
    IResult<OdrBw> get_odrbw();

    IResult<> calibrate(uint8_t samples);
    IResult<> update();

    Option<Vector3_t<q24>> read_acc();
    IResult<uint8_t> read_temperature();
private:
    hal::I2cDrv i2c_drv_;

    
    IResult<> write_reg(uint8_t reg, uint8_t value){
        if(const auto res = i2c_drv_.write_reg(reg, value);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_reg(uint8_t reg, uint8_t & ret){
        if(const auto res = i2c_drv_.read_reg(reg, ret);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_burst(uint8_t reg, uint8_t *buf, uint8_t size){
        if(const auto res = i2c_drv_.read_burst(reg, std::span(buf, size), LSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
};

}

