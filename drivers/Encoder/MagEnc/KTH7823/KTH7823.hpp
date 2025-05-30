#pragma once

#include "core/io/regs.hpp"
#include "drivers/Encoder/MagEncoder.hpp"

#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

struct KTH7823_Collections{
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

struct KTH7823_Regs:public KTH7823_Collections{

    struct R8_Z_low:public Reg8<>{
        static constexpr RegAddress address = 0x00;
        uint8_t z_high;
    }DEF_R8(z_low_reg)

    struct R8_Z_high:public Reg8<>{
        static constexpr RegAddress address = 0x01;
        uint8_t z_high;
    }DEF_R8(z_high_reg)

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
        uint8_t mag_low:3;
        uint8_t mag_high:3;
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

class KTH7823_Phy final:public KTH7823_Collections{ 
public:
    KTH7823_Phy(hal::SpiDrv && spi_drv):spi_drv_(spi_drv){}
    KTH7823_Phy(hal::Spi & spi, const hal::SpiSlaveIndex idx):
        spi_drv_(hal::SpiDrv(spi, idx)){}

    [[nodiscard]] IResult<uint16_t> direct_read(){
        uint16_t rx;
        if(const auto res = transceive_u16(rx, 0);
            res.is_err()) return Err(res.unwrap_err());

        return Ok(rx);
    }

    [[nodiscard]] IResult<uint8_t> read_reg(const uint8_t addr){
        const uint16_t tx = (uint16_t((addr & 0b00'111111) | 0b01'000000) << 8);
        uint16_t rx;
        const auto res = transceive_u16(rx, tx);
        if(res.is_err()) return Err(res.unwrap_err());
        return Ok(rx >> 8);
    }

    [[nodiscard]] IResult<> burn_reg(const uint8_t addr, const uint8_t data){
        const uint16_t tx = (uint16_t((addr & 0b00'111111) | 0b10'000000) << 8) | data;
        uint16_t rx;
        const auto res = transceive_u16(rx, tx);
        if(res.is_err()) return Err(res.unwrap_err());
        if((rx >> 8) != data) return Err(Error::RegProgramFailed);
        return Ok();
    }

    [[nodiscard]] IResult<> disable_reg_oper(){
        uint16_t dummy;
        return transceive_u16(dummy, 0b1110'1000'0000'0010);
    }

    [[nodiscard]] IResult<> enable_reg_oper(){
        uint16_t dummy;
        return transceive_u16(dummy, 0b1110'1000'0000'0000);
    }
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
    KTH7823(const Phy && phy):phy_(std::move(phy)){;}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> set_zero_position(const real_t position);
    [[nodiscard]] IResult<real_t> get_lap_position(){
        return Ok(lap_position_);
    }

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