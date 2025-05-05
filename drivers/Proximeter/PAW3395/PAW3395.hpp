#pragma once

//这个驱动还未完成
//这个驱动还未测试

//PAW3395 是原相科技的一款游戏级顶级鼠标光流传感器

// https://github.com/ttwards/motor/blob/939f1db78dcaae6eb819dcb54b6146d94db7dffc/drivers/sensor/paw3395/paw3395.h#L122

#include "details/PAW3395_collections.hpp"

namespace ymd::drivers{

class PAW3395 final:public FlowSensorIntf, public PAW3395_Collections{
public:

public:
    PAW3395(const PAW3395 & other) = delete;
    PAW3395(PAW3395 && other) = delete;

    PAW3395(const hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    PAW3395(hal::SpiDrv && spi_drv):spi_drv_(std::move(spi_drv)){;}
    PAW3395(hal::Spi & spi, const hal::SpiSlaveIndex index):spi_drv_(hal::SpiDrv(spi, index)){;}

    [[nodiscard]] IResult<> corded_gaming();

    [[nodiscard]] IResult<bool> validate();
    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<> update(const real_t rad);

    [[nodiscard]] Vector2_t<real_t> get_position(){
        return {x_cm * real_t(0.01), y_cm * real_t(0.01)};
    }

    [[nodiscard]] IResult<> set_led(bool on);

    IResult<> set_dpi(uint16_t DPI_Num);

    IResult<> set_lift_off(bool height);
private:
    hal::SpiDrv spi_drv_;

    
    real_t x_cm = {};
    real_t y_cm = {};


    [[nodiscard]] IResult<> write_reg(const uint8_t addr, const uint8_t data){
        const std::array<uint8_t, 2> temp = {
            uint8_t(addr| 0x80),
            data
        };

        if(const auto res = spi_drv_.write_burst<uint8_t>(temp.data(), temp.size());
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<uint8_t> read_reg(const uint8_t addr){
        const std::array<uint8_t, 2> pbuf_tx = {
            uint8_t(addr| 0x80),
            0x00
        };

        std::array<uint8_t, 2> pbuf_rx;
        if(const auto res = spi_drv_.transceive_burst<uint8_t, 2>(
            std::span(pbuf_rx), 
            std::span(pbuf_tx)
        ); res.is_err()) return Err(res.unwrap_err());
        return Ok(pbuf_rx[1]);
    }

    [[nodiscard]] IResult<int16_t> read_i16(const uint8_t addr1, const uint8_t addr2){
        const uint8_t low_byte = ({
            const auto res = read_reg(addr1);
            if(res.is_err()) return Err(res.unwrap_err());
            res.unwrap();
        });

        const uint8_t high_byte = ({
            const auto res = read_reg(addr2);
            if(res.is_err()) return Err(res.unwrap_err());
            res.unwrap();
        });

        return Ok(int16_t((high_byte << 8) | low_byte));
    }

    [[nodiscard]] IResult<> write_list(std::span<const std::pair<uint8_t, uint8_t>> list){
        for(const auto & [addr, data] : list){
            if(const auto res = write_reg(addr, data); res.is_err()) return res;
        }
        return Ok();
    }

    [[nodiscard]] IResult<> powerup();
};

}