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


#include "mlx90640_prelude.hpp"

namespace ymd::drivers{

struct MLX90640:public MLX90640_Prelude{
public:

    template<typename T = void>
    using IResult = Result<T, Error>;

    explicit MLX90640(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit MLX90640(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit MLX90640(
        Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(i2c, addr){;}

    [[nodiscard]] IResult<> init(uint16_t EE[832], MLX90640_Coeffs & MLXPars);
    
    [[nodiscard]] IResult<> dump_ee(uint16_t *eeData);
    [[nodiscard]] IResult<uint16_t> get_frame_data(uint16_t *frameData);
    [[nodiscard]] IResult<> extract_parameters(const uint16_t *eeData, MLX90640_Coeffs *mlx90640);
    [[nodiscard]] IResult<> set_resolution(uint8_t resolution);
    [[nodiscard]] IResult<uint16_t> get_cur_resolution();
    [[nodiscard]] IResult<> set_refresh_rate(DataRate rate);   
    [[nodiscard]] IResult<uint16_t> get_cur_mode(); 
    [[nodiscard]] IResult<> set_interleaved_mode();
    [[nodiscard]] IResult<> set_chess_mode();
    [[nodiscard]] IResult<uint16_t> get_refresh_rate(); 
    float get_ta(const uint16_t *frameData, const MLX90640_Coeffs *params);
    void get_image(const uint16_t *frameData, const MLX90640_Coeffs *params, float *result);
    void calculate_to(const uint16_t *frameData, const MLX90640_Coeffs *params, float emissivity, float tr, float *result);
private:
    hal::I2cDrv i2c_drv_;

    IResult<> read_burst(uint16_t startAddress, std::span<uint16_t> pbuf){

        if(const auto res = i2c_drv_.read_burst<uint16_t>(startAddress, pbuf, std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> write_reg(uint16_t writeAddress, uint16_t data){
        if(const auto res = i2c_drv_.write_reg<uint16_t>(writeAddress, data, std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

};



}