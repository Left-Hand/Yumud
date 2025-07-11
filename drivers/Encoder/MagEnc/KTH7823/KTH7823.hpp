#pragma once

//KTH7823是一款16位磁编码器

#include "core/io/regs.hpp"
#include "drivers/Encoder/MagEncoder.hpp"

#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

struct KTH7823_Prelude{
    using Error = EncoderError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class Width:uint8_t{
        W90, W180, W270, W360
    };

    enum class Phase:uint8_t{
        P0, P90, P180, P270
    };

    enum class MagThreshold:uint8_t{
        mT23, mT38, mT53, mT67, mT81, mT95, mT109, mT123
    };

    using RegAddress = uint8_t;

};

struct KTH7823_Regs:public KTH7823_Prelude{

    struct R8_Zero_low:public Reg8<>{
        static constexpr RegAddress address = 0x00;
        uint8_t data;
    }DEF_R8(zero_low_reg)

    struct R8_Zero_high:public Reg8<>{
        static constexpr RegAddress address = 0x01;
        uint8_t data;
    }DEF_R8(zero_high_reg)

    struct R8_GainTrim:public Reg8<>{
        static constexpr RegAddress address = 0x02;
        uint8_t gain_trim;
    }DEF_R8(gain_trim_reg)

    struct R8_XyTraim:public Reg8<>{
        static constexpr RegAddress address = 0x03;
        uint8_t x_trim:1;
        uint8_t y_trim:1;
        uint8_t __resv__:6;
    }DEF_R8(xy_trim_reg)

    struct R8_Z_Config:public Reg8<>{
        static constexpr RegAddress address = 0x04;
        uint8_t __resv__:2;
        uint8_t zd:2;
        uint8_t zl:2;
        uint8_t ppt:2;
    }DEF_R8(z_config_reg)

    struct R8_PPT_High:public Reg8<>{
        static constexpr RegAddress address = 0x05;
        uint8_t ppt_high;
    }DEF_R8(ppt_high_reg)

    struct R8_MagAlert:public Reg8<>{
        static constexpr RegAddress address = 0x06;
        uint8_t __resv__:2;
        MagThreshold mag_low:3;
        MagThreshold mag_high:3;
    }DEF_R8(mag_alert_reg)

    struct R8_Npp:public Reg8<>{
        static constexpr RegAddress address = 0x07;
        uint8_t __resv__:5;
        uint8_t npp:3;
    }DEF_R8(npp_reg)

    struct R8_AbzLimit:public Reg8<>{
        static constexpr RegAddress address = 0x08;
        uint8_t abz_limit;
    }DEF_R8(abz_limit_reg)

    struct R8_Rd:public Reg8<>{
        static constexpr RegAddress address = 0x09;
        uint8_t __resv__:7;
        uint8_t rd:1;
    }DEF_R8(rd_reg)
};

class KTH7823_Phy final:public KTH7823_Prelude{ 
public:
    KTH7823_Phy(hal::SpiDrv && spi_drv):spi_drv_(spi_drv){}
    KTH7823_Phy(Some<hal::Spi *> spi, const hal::SpiSlaveIndex idx):
        spi_drv_(hal::SpiDrv(spi, idx)){}

    [[nodiscard]] IResult<uint16_t> direct_read();

    [[nodiscard]] IResult<uint8_t> read_reg(const uint8_t addr);

    [[nodiscard]] IResult<> burn_reg(const uint8_t addr, const uint8_t data);

    template<typename T>
    [[nodiscard]] IResult<> burn_reg(const RegCopy<T> & reg){
        if(const auto res = burn_reg(reg.address, reg.as_val()); 
            res.is_err()) return res;
        reg.apply();
        return Ok();
    }

    [[nodiscard]] IResult<> disable_reg_oper();

    [[nodiscard]] IResult<> enable_reg_oper();
private:
    hal::SpiDrv spi_drv_;

    [[nodiscard]] IResult<> transceive_u16(uint16_t & rx, const uint16_t tx);
};

class KTH7823 final:
    public MagEncoderIntf, 
    public KTH7823_Regs{
public:
    using Phy = KTH7823_Phy;
public:
    KTH7823(const Phy && phy):
        phy_(std::move(phy)){;}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<real_t> read_lap_position(){
        return Ok(lap_position_);
    }

    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> set_zero_position(const real_t position);

    [[nodiscard]] IResult<> set_trim_x(const real_t k);

    [[nodiscard]] IResult<> set_trim_y(const real_t k);

    [[nodiscard]] IResult<> set_trim(const real_t am, const real_t e);

    [[nodiscard]] IResult<> set_mag_threshold(const MagThreshold low, const MagThreshold high);

    [[nodiscard]] IResult<> set_direction(const bool direction);

    [[nodiscard]] IResult<MagStatus> get_mag_status();

    [[nodiscard]] IResult<> set_zparameters(const Width width, const Phase phase);

    [[nodiscard]] IResult<> set_pulse_per_turn(const uint16_t ppt);

private:

    Phy phy_;
    real_t lap_position_ = {};

    [[nodiscard]]
    IResult<> write_reg(const RegAddress addr, uint8_t data);

    [[nodiscard]]
    IResult<> read_reg(const RegAddress addr, uint8_t & reg);

    [[nodiscard]]
    IResult<> direct_read(uint16_t & data);
    
    [[nodiscard]]
    IResult<uint16_t> get_raw_data();
    
    [[nodiscard]]
    IResult<> set_zero_data(const uint16_t data);
};

};