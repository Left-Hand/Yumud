#pragma once

// 相当优秀的磁力计 调整好一点不带漂

// 这个驱动已经完成编写
// 这个驱动已经完成测试

// https://www.memsic.com/Public/Uploads/uploadfile/files/20220119/MMC5983MADatasheetRevA.pdf
// https://github.com/kriswiner/MMC5983MA/blob/master/LSM6DSM_MMC5983MA_LPS22HB_Dragonfly/MMC5983MA.cpp

#include "mmc5983_prelude.hpp"

namespace ymd::drivers{


class MMC5983:
    public MagnetometerIntf,
    public MMC5983_Regs{
public:
    struct Config{
        PrdSet prd_set = PrdSet::_100;
        BandWidth bandwidth = BandWidth::_200Hz;
        Odr data_rate = Odr::_200Hz;
    };

    explicit MMC5983(const hal::I2cDrv & i2c_drv):
        phy_(i2c_drv){;}
    explicit MMC5983(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        phy_(hal::I2cDrv{i2c, addr}){;}
    explicit MMC5983(const hal::SpiDrv & spi_drv):
        phy_(spi_drv){;}
    explicit MMC5983(Some<hal::Spi *> spi, const hal::SpiSlaveRank index):
        phy_(hal::SpiDrv{spi, index}){;}

    [[nodiscard]] IResult<> init(const Config & cfg);
    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<> set_odr(const Odr odr);
    [[nodiscard]] IResult<> set_bandwidth(const BandWidth bw);
    [[nodiscard]] IResult<> enable_x(const Enable en);
    [[nodiscard]] IResult<> enable_yz(const Enable en);
    
    [[nodiscard]] IResult<Vec3<q24>> read_mag();
    [[nodiscard]] IResult<q16> read_temp();
    
    [[nodiscard]] IResult<bool> is_mag_meas_done();
    [[nodiscard]] IResult<bool> is_temp_meas_done();
    
    
    [[nodiscard]] IResult<> set_prd_magset(const PrdSet prdset);
    [[nodiscard]] IResult<> enable_magset(const Enable en);
    [[nodiscard]] IResult<> enable_magreset(const Enable en);
    [[nodiscard]] IResult<Vec3<q24>> do_magset();
    [[nodiscard]] IResult<Vec3<q24>> do_magreset();
    [[nodiscard]] IResult<> enable_auto_mag_sr(const Enable en);
    
    [[nodiscard]] IResult<> enable_mag_meas(const Enable en);
    [[nodiscard]] IResult<> enable_temp_meas(const Enable en);
private:    
    using Phy = MMC5983_Phy;
    Phy phy_;

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        const auto res = phy_.write_reg(reg.address, reg.as_val());
        if(res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return res;
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return phy_.read_reg(reg.address, reg.as_ref());
    }

    [[nodiscard]] IResult<> read_burst(const uint8_t addr, std::span<uint8_t> pbuf){
        return phy_.read_burst(addr, pbuf);
    }

};

}