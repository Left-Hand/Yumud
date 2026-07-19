#pragma once

//这个驱动还未完成
//这个驱动还未测试

//PAW3395 是原相科技的一款游戏级顶配鼠标光流传感器


#include "paw3395_prelude.hpp"

namespace ymd::drivers::paw3395{

class PAW3395 final:
    public PAW3395_Prelude{
public:
    explicit PAW3395(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    explicit PAW3395(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}
    explicit PAW3395(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        spi_drv_(hal::SpiDrv(spi, rank)){;}

    PAW3395(const PAW3395 & other) = delete;
    PAW3395(PAW3395 && other) = delete;
    ~PAW3395() = default;

    struct Config{
        uint16_t dpi_num;
    };

    IResult<> init(const Config & cfg);

    IResult<> corded_gaming();

    IResult<> validate();

    IResult<math::Vec2i> update();

    IResult<bool> is_motioned();

    IResult<> enable_ripple(const Enable en);

    IResult<> set_led(bool on);

    IResult<> set_dpi(uint16_t dpi_num);

    IResult<> set_lift_off(bool height);
private:
    hal::SpiDrv spi_drv_;

    IResult<math::Vec2i> query_xy();
    IResult<> write_reg(const RegAddr addr, const uint8_t RegAddr);

    IResult<> read_reg(const RegAddr addr, uint8_t & data);

    IResult<int16_t> read_i16(const RegAddr low_addr, const RegAddr high_addr);

    IResult<> write_list(
        std::span<const std::pair<uint8_t, uint8_t>> list);

    IResult<> powerup();
};

}