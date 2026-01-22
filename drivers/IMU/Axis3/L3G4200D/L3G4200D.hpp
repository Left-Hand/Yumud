

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

#pragma once

#include "core/io/regs.hpp"
#include "core/utils/result.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "drivers/IMU/IMU.hpp"


namespace ymd::drivers{

struct L3G4200D_Prelude{
public:
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0xD0 >> 1);

    using Error = ImuError;
    
    template<typename T = void>
    using IResult = Result<T, Error>;


    enum class [[nodiscard]] Dps:uint8_t{
        _2000DPS = 0b10,
        _500DPS  = 0b01,
        _250DPS  = 0b00
    };

    enum class [[nodiscard]] OdrBw:uint8_t{
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
};


struct L3G4200D_Regs:public L3G4200D_Prelude{

};

class L3G4200D:
    public L3G4200D_Prelude{
public:
    explicit L3G4200D(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR)
        :i2c_drv_(hal::I2cDrv(i2c, addr)){}
    explicit L3G4200D(hal::I2cDrv && i2c_drv):
        i2c_drv_(i2c_drv){}

    struct Config{
        Dps dps;
        OdrBw odrbw;
    };

    [[nodiscard]] IResult<> init(const Config & cfg);
    [[nodiscard]] IResult<> reconf(const Config & cfg);

    [[nodiscard]] IResult<Dps> get_dps();
    [[nodiscard]] IResult<OdrBw> get_odrbw();

    [[nodiscard]] IResult<> calibrate(uint8_t samples);
    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<Vec3<iq24>> read_acc();
    [[nodiscard]] IResult<uint8_t> read_temperature();
private:
    using Regs = L3G4200D_Regs;
    Regs regs_ = {};
    hal::I2cDrv i2c_drv_;

    [[nodiscard]] IResult<> write_reg(uint8_t reg_addr, uint8_t reg_val){
        if(const auto res = i2c_drv_.write_reg(reg_addr, reg_val);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_reg(uint8_t reg_addr, uint8_t & reg_val){
        if(const auto res = i2c_drv_.read_reg(reg_addr, reg_val);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_burst(uint8_t reg, std::span<uint8_t> pbuf){
        if(const auto res = i2c_drv_.read_burst(reg, pbuf, std::endian::little);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
};

}

