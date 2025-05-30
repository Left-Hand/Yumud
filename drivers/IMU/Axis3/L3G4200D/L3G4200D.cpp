// Copyright 2021 IOsetting <iosetting(at)outlook.com>
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

#include "L3G4200D.hpp"

static constexpr uint8_t ADDRESS = 0xD2; // SDO->High:11010010(0xD2), SDO->Low:11010000(0xD0)

static constexpr uint8_t REG_WHO_AM_I = 0x0F; // Fixed value: 11010011(D3)

static constexpr uint8_t REG_CTRL_REG1 = 0x20; // [0,2]:X,Y,Z axis enable
                                          // [3]Power down mode
                                          // [4,5]:Bandwidth
                                          // [6,7]:Output Data Rate

static constexpr uint8_t REG_CTRL_REG2 = 0x21; // [0,3]:High Pass filter Cut Off frequency, 
                                          // [4,5]:High Pass filter Mode

static constexpr uint8_t REG_CTRL_REG3 = 0x22; // [0]:FIFO Empty interrupt on DRDY/INT2. Default value: 0. (0: Disable; 1: Enable)
                                          // [1]:FIFO Overrun interrupt on DRDY/INT2 Default value: 0. (0: Disable; 1: Enable)
                                          // [2]:FIFO Watermark interrupt on DRDY/INT2. Default value: 0. (0: Disable; 1: Enable)
                                          // [3]:Date Ready on DRDY/INT2. Default value 0. (0: Disable; 1: Enable)
                                          // [4]:Push- Pull / Open drain. Default value: 0. (0: Push- Pull; 1: Open drain)
                                          // [5]:Interrupt active configuration on INT1. Default value 0. (0: High; 1:Low)
                                          // [6]:Boot status available on INT1. Default value 0. (0: Disable; 1: Enable)
                                          // [7]:Interrupt enable on INT1 pin. Default value 0. (0: Disable; 1: Enable)

static constexpr uint8_t REG_CTRL_REG4 = 0x23; // [0]:SPI Serial Interface Mode, (0: 4-wire, 1: 3-wire)
                                          // [1,2]:Self Test Enable. (00: Self Test Disabled)
                                          // [4,5]:Full Scale selection. (00: 250 dps; 01: 500 dps; 10: 2000 dps; 11: 2000 dps)
                                          // [6]:Big/Little Endian Data Selection. (0: Data LSB @ lower address; 1: Data MSB @ lower address)
                                          // [7]:Block Data Update. (0: continous update; 1: output registers not updated until MSB and LSB reading)

static constexpr uint8_t REG_CTRL_REG5 = 0x24; // [0,1]:Out selection configuration
                                          // [2,3]:INT1 selection configuration
                                          // [4]:High Pass filter Enable, (0: HPF disabled; 1: HPF enabled)
                                          // [6]:FIFO enable. (0: FIFO disable; 1: FIFO Enable)
                                          // [7]:Reboot memory content. (0: normal mode; 1: reboot memory content)

static constexpr uint8_t REG_REFERENCE = 0x25; // Reference value for Interrupt generation. Default value: 0

static constexpr uint8_t REG_OUT_TEMP = 0x26; // Temperature data

static constexpr uint8_t REG_STATUS_REG = 0x27; // [0]:X axis new data available. 0:no, 1:yes
                                          // [1]:Y axis new data available. 0:no, 1:yes
                                          // [2]:Z axis new data available. 0:no, 1:yes
                                          // [3]:X, Y, Z -axis new data available. 0:no, 1:yes
                                          // [4]:X axis data overrun. 1: new data has overwritten the previous one
                                          // [5]:Y axis data overrun. 1: new data has overwritten the previous one
                                          // [6]:Z axis data overrun. 1: new data has overwritten the previous one
                                          // [7]:X, Y, Z -axis data overrun. 1: new data has overwritten the previous one

static constexpr uint8_t REG_OUT_X_L = 0x28; // X-axis angular rate data
static constexpr uint8_t REG_OUT_X_H = 0x29; // X-axis angular rate data
static constexpr uint8_t REG_OUT_Y_L = 0x2A; // Y-axis angular rate data
static constexpr uint8_t REG_OUT_Y_H = 0x2B; // Y-axis angular rate data
static constexpr uint8_t REG_OUT_Z_L = 0x2C; // Z-axis angular rate data
static constexpr uint8_t REG_OUT_Z_H = 0x2D; // Z-axis angular rate data

static constexpr uint8_t REG_FIFO_CTRL_REG = 0x2E; // [0,4]:FIFO threshold. Watermark level setting
                                          // [5,7]:FIFO mode selection, 000:Bypass mode, 001:FIFO mode, 010:Stream mode, 011:Stream-to-FIFO mode, 100:Bypass-to-Stream mode

static constexpr uint8_t REG_FIFO_SRC_REG = 0x2F; // [0,4]:FIFO stored data level
                                          // [5]:FIFO empty bit. ( 0: FIFO not empty; 1: FIFO empty)
                                          // [6]:Overrun bit status. (0: FIFO is not completely filled; 1:FIFO is completely filled)
                                          // [7]:Watermark status. (0: FIFO filling is lower than WTM level; 1: FIFO filling is equalor higher than WTM level)

static constexpr uint8_t REG_INT1_CFG = 0x30; // [0]:X interrupt on measured accel. lower than preset threshold (0: disable, 1: enable)
                                          // [1]:X interrupt on measured accel. higher than preset threshold (0: disable, 1: enable)
                                          // [2]:Y interrupt on measured accel. lower than preset threshold (0: disable, 1: enable)
                                          // [3]:Y interrupt on measured accel. higher than preset threshold (0: disable, 1: enable)
                                          // [4]:Z interrupt on measured accel. lower than preset threshold (0: disable, 1: enable)
                                          // [5]:Z interrupt on measured accel. higher than preset threshold (0: disable, 1: enable)
                                          // [6]:Latch Interrupt Request. (0:not latched; 1: latched) Cleared by reading INT1_SRC reg.
                                          // [7]:AND/OR combination of Interrupt events. Default value: 0, (0: OR, 1: AND)

static constexpr uint8_t REG_INT1_SRC = 0x31; // [0]:Interrupt source X low
                                          // [1]:Interrupt source X high
                                          // [2]:Interrupt source Y low
                                          // [3]:Interrupt source Y high
                                          // [4]:Interrupt source Z low
                                          // [5]:Interrupt source Z high
                                          // [6]:Interrupt active. (0: no interrupt, 1: one or more interrupts have been generated)

static constexpr uint8_t REG_INT1_THS_XH = 0x32; // [0,7]:Interrupt threshold X
static constexpr uint8_t REG_INT1_THS_XL = 0x33; // [0,7]:Interrupt threshold X
static constexpr uint8_t REG_INT1_THS_YH = 0x34; // [0,7]:Interrupt threshold Y
static constexpr uint8_t REG_INT1_THS_YL = 0x35; // [0,7]:Interrupt threshold Y
static constexpr uint8_t REG_INT1_THS_ZH = 0x36; // [0,7]:Interrupt threshold Z
static constexpr uint8_t REG_INT1_THS_ZL = 0x37; // [0,7]:Interrupt threshold Z

static constexpr uint8_t REG_INT1_DURATION = 0x38; // [0,6]:The minimum duration of the Interrupt event to be recognized. Duration steps and maximum values depend on the ODR chosen.
                                          // [7]:WAIT enable. (0: disable; 1: enable)
                                          //     Wait =’1’: if signal crosses the selected threshold, the interrupt falls only after the duration
                                          //     has counted number of samples at the selected data rate, written into the duration counter register.

static constexpr uint8_t ENABLE_X                   = 0b00000001;
static constexpr uint8_t ENABLE_Y                   = 0b00000010;
static constexpr uint8_t ENABLE_Z                   = 0b00000100;
static constexpr uint8_t ENABLE_ALL                 = 0b00000111;
static constexpr uint8_t ENABLE_NONE                = 0b00000000;
static constexpr uint8_t POWER_ON                   = 0b00001000;

static constexpr auto DPS_250                    = real_t(.00875f);
static constexpr auto DPS_500                    = real_t(.0175f);
static constexpr auto DPS_2000                   = real_t(.07f);


using namespace ymd;
using namespace ymd::drivers;

using Error = L3G4200D::Error;
using Dps = L3G4200D::Dps;
using OdrBw = L3G4200D::OdrBw;
using Config = L3G4200D::Config;


template<typename T = void>
using IResult = Result<T, Error>;


IResult<> L3G4200D::reconf(const Config & cfg){
    // Check L3G4200D Who Am I Register

    static constexpr uint8_t KEY_WHO_AM_I = 0xD3;

    uint8_t temp;
    if (const auto res = read_reg(REG_WHO_AM_I, temp); res.is_ok()){
        if (temp != KEY_WHO_AM_I) return Err(Error::WrongWhoAmI);
    }else return Err(res.unwrap_err());
    // Enable all axis and setup normal mode
    if(const auto res = write_reg(REG_CTRL_REG1, (static_cast<uint8_t>(
            cfg.odrbw) << 4) | ENABLE_ALL | POWER_ON);
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = write_reg(REG_CTRL_REG4, static_cast<uint8_t>(
            cfg.dps) << 4);
        res.is_err()) return Err(res.unwrap_err());
    
    return Ok();
}

// IResult<Dps> L3G4200D::get_dps(){
//     uint8_t ret;
//     if(const auto res = read_reg(REG_CTRL_REG4);
//         res.is_err()) return Err(res.unwrap_err());
//     return Ok(static_cast<Dps>(ret >> 4) & 0x03);
// }

// IResult<OdrBw> L3G4200D::get_odr_bw(){
//     uint8_t ret;
//     (read_reg(REG_CTRL_REG1, ret)
//     return static_cast<>(ret>> 4) & 0x0F);
// }


IResult<> L3G4200D::calibrate(uint8_t samples){
    // int32_t sum[3] = {0,0,0}, sigma[3] = {0,0,0};

    // // Read samples
    // for (uint8_t i = 0; i < samples; ++i)
    // {
    //     read_reg(delta);
    //     sum[0] += delta[0];
    //     sum[1] += delta[1];
    //     sum[2] += delta[2];

    //     sigma[0] += delta[0] * delta[0];
    //     sigma[1] += delta[1] * delta[1];
    //     sigma[2] += delta[2] * delta[2];

    //     clock::delay(5ms);
    // }
    // // Calculate delta
    // delta[0] = sum[0] / samples;
    // delta[1] = sum[1] / samples;
    // delta[2] = sum[2] / samples;
    // // Calculate threshold
    // threshold[0] = sqrt((sigma[0] / samples) - (delta[0] * delta[0]));
    // threshold[1] = sqrt((sigma[1] / samples) - (delta[1] * delta[1]));
    // threshold[2] = sqrt((sigma[2] / samples) - (delta[2] * delta[2]));
    TODO();
    return Ok();
}

IResult<> L3G4200D::update(){
    TODO();
    return Ok();
}


IResult<Vector3<q24>> L3G4200D::read_acc(){
    return Ok(Vector3<q24>());
}

IResult<uint8_t> L3G4200D::read_temperature(){
    // uint8_t ret;
    // return read_reg(ret);
    TODO();
    return Ok(0);
}