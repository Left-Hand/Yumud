#pragma once

#include "mt6835_prelude.hpp"

namespace ymd::drivers{

class MT6835 final:
    public MagEncoderIntf,
    public MT6835_Regs{
public:
    explicit MT6835(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    explicit MT6835(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}
    explicit MT6835(Some<hal::Spi *> spi, const hal::SpiSlaveRank index):
        spi_drv_(hal::SpiDrv{spi, index}){;}

    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<Angle<q31>> read_lap_angle(){
        return Ok(Angle<q31>::from_turns(lap_position_));
    }
    [[nodiscard]] uint32_t get_err_cnt() const {return errcnt_;}
private:
    hal::SpiDrv spi_drv_;

    real_t lap_position_ = 0;
    size_t errcnt_ = 0;
    bool fast_mode_ = true;

    uint16_t get_position_data();

    [[nodiscard]] IResult<> write_reg(const RegAddr addr, const uint8_t data);
    [[nodiscard]] IResult<> read_reg(const RegAddr addr, uint8_t & data);
};

};