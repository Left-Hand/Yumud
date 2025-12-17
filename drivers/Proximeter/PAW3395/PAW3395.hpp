#pragma once

//这个驱动还未完成
//这个驱动还未测试

//PAW3395 是原相科技的一款游戏级顶配鼠标光流传感器


#include "details/PAW3395_Prelude.hpp"

namespace ymd::drivers::paw3395{

class PAW3395 final:
    public PAW3395_Prelude{
public:
    explicit PAW3395(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    explicit PAW3395(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}
    explicit PAW3395(Some<hal::SpiBase *> spi, const hal::SpiSlaveRank rank):
        spi_drv_(hal::SpiDrv(spi, rank)){;}

    PAW3395(const PAW3395 & other) = delete;
    PAW3395(PAW3395 && other) = delete;
    ~PAW3395() = default;

    struct Config{
        uint16_t dpi_num;
    };

    [[nodiscard]] IResult<> init(const Config & cfg);

    [[nodiscard]] IResult<> corded_gaming();

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<Vec2i> update();

    [[nodiscard]] IResult<bool> is_motioned();

    [[nodiscard]] IResult<> enable_ripple(const Enable en);

    [[nodiscard]] IResult<> set_led(bool on);

    [[nodiscard]] IResult<> set_dpi(uint16_t dpi_num);

    [[nodiscard]] IResult<> set_lift_off(bool height);
private:
    hal::SpiDrv spi_drv_;

    [[nodiscard]] IResult<Vec2i> query_xy();
    [[nodiscard]] IResult<> write_reg(const RegAddr addr, const uint8_t RegAddr);

    [[nodiscard]] IResult<> read_reg(const RegAddr addr, uint8_t & data);

    [[nodiscard]] IResult<int16_t> read_i16(const RegAddr low_addr, const RegAddr high_addr);

    [[nodiscard]] IResult<> write_list(
        std::span<const std::pair<uint8_t, uint8_t>> list);

    [[nodiscard]] IResult<> powerup();
};

}