#pragma once

//这个驱动还未完成
//这个驱动还未测试

//PAW3395 是原相科技的一款游戏级顶级鼠标光流传感器

// https://github.com/ttwards/motor/blob/939f1db78dcaae6eb819dcb54b6146d94db7dffc/drivers/sensor/paw3395/paw3395.h#L122

#include "details/PAW3395_Prelude.hpp"

namespace ymd::drivers{

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
    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<> corded_gaming();

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<Vec2i> sample_fetch();

    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> enable_ripple(const Enable en);

    [[nodiscard]] IResult<> set_led(bool on);

    [[nodiscard]] IResult<> set_dpi(uint16_t DPI_Num);

    [[nodiscard]] IResult<> set_lift_off(bool height);
private:
    hal::SpiDrv spi_drv_;

    [[nodiscard]] IResult<> write_reg(const uint8_t addr, const uint8_t data);

    [[nodiscard]] IResult<> read_reg(const uint8_t addr, uint8_t & data);

    [[nodiscard]] IResult<int16_t> read_i16(const uint8_t low_addr, const uint8_t high_addr);

    [[nodiscard]] IResult<> write_list(
        std::span<const std::pair<uint8_t, uint8_t>> list);

    [[nodiscard]] IResult<> powerup();
};

}