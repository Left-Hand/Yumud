/**
 * @copyright (C) 2017 Melexis N.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#pragma once


#include <stdint.h>

#include "hal/bus/i2c/i2cdrv.hpp"

#include "core/utils/Result.hpp"
#include "core/utils/errno.hpp"


namespace ymd::drivers{

enum class MLX90640_ErrorKind:uint8_t{
    BrokenPixelsNum = 3,
    OutlierPixelsNum = 4,
    BadPixelsNum = 5,
    AdjacentBadPixels = 6,
    EepromDataErr = 7,
    FrameDataErr = 8,
    MeasTriggerErr = 9
};

DEF_DERIVE_DEBUG(MLX90640_ErrorKind)
DEF_ERROR_SUMWITH_HALERROR(MLX90640_Error, MLX90640_ErrorKind)

    
struct paramsMLX90640{
    int16_t kVdd;
    int16_t vdd25;
    float KvPTAT;
    float KtPTAT;
    uint16_t vPTAT25;
    float alphaPTAT;
    int16_t gainEE;
    float tgc;
    float cpKv;
    float cpKta;
    uint8_t resolutionEE;
    uint8_t calibrationModeEE;
    float KsTa;
    float ksTo[5];
    int16_t ct[5];
    uint16_t alpha[768];    
    uint8_t alphaScale;
    int16_t offset[768];    
    int8_t kta[768];
    uint8_t ktaScale;    
    int8_t kv[768];
    uint8_t kvScale;
    float cpAlpha[2];
    int16_t cpOffset[2];
    float ilChessC[3]; 
    uint16_t brokenPixels[5];
    uint16_t outlierPixels[5];  
} ;

struct MLX90640{
public:
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x33);

    enum class DataRate:uint8_t{
        _0_5Hz = 0,
        _1Hz = 1,
        _2Hz = 2,
        _4Hz = 3,
        _8Hz = 4,
        _16Hz = 5,
        _32Hz = 6,
        _64Hz = 7
    };


    explicit MLX90640(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit MLX90640(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit MLX90640(
        Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(i2c, addr){;}



    template<typename T = void>
    using IResult = Result<T, MLX90640_Error>;


    IResult<> init(uint16_t EE[832], paramsMLX90640 & MLXPars);
    
    IResult<> dump_ee(uint16_t *eeData);
    IResult<> synch_frame(uint8_t slaveAddr);
    IResult<uint16_t> get_frame_data(uint16_t *frameData);
    IResult<> extract_parameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
    IResult<> set_resolution(uint8_t resolution);
    IResult<uint16_t> get_cur_resolution(uint8_t slaveAddr);
    IResult<> set_refresh_rate(DataRate rate);   
    IResult<uint16_t> get_cur_mode(uint8_t slaveAddr); 
    IResult<> set_interleaved_mode(uint8_t slaveAddr);
    IResult<> set_chess_mode(uint8_t slaveAddr);
    IResult<uint16_t> get_refresh_rate(uint8_t slaveAddr); 
    float get_ta(uint16_t *frameData, const paramsMLX90640 *params);
    void get_image(uint16_t *frameData, const paramsMLX90640 *params, float *result);
    void calculate_to(uint16_t *frameData, const paramsMLX90640 *params, float emissivity, float tr, float *result);

    int get_subpage_number(uint16_t *frameData);

    void bad_pixels_correction(uint16_t *pixels, float *to, int mode, paramsMLX90640 *params);


    IResult<> I2CRead(uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data){

        if(const auto res = i2c_drv_.read_burst<uint16_t>(startAddress, 
            std::span<uint16_t>((data), size_t(nMemAddressRead)), MSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> I2CWrite(uint16_t writeAddress, uint16_t data){
        if(const auto res = i2c_drv_.write_reg<uint16_t>(writeAddress, data, MSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

private:
    hal::I2cDrv i2c_drv_;
};



}