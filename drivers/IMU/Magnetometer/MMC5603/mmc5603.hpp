#pragma once

#include "core/io/regs.hpp"
#include "drivers/IMU/IMU.hpp"


#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

struct MMC5603_Prelude{

    using Error = ImuError;
    template<typename T = void>
    using IResult = Result<T, Error>;
    
    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b01100000 >> 1);

    enum class DataRate:uint8_t{
        _75,
        _150,
        _255,
        _1000
    };

    enum class BandWidth:uint8_t{
        _6_6ms,
        _3_5ms,
        _2_0ms,
        _1_2ms
    };
    
};


struct MMC5603_Regs:public MMC5603_Prelude { 
    using RegAddress = uint8_t;

    struct AxisReg:public Reg16<>{
        scexpr RegAddress address_x = 0x00;
        scexpr RegAddress address_y = 0x02;
        scexpr RegAddress address_z = 0x04;

        uint8_t data_h;
        uint8_t data_l;
    };

    struct ExtAxisReg:public Reg8<>{
        scexpr RegAddress address_x = 0x06;
        scexpr RegAddress address_y = 0x07;
        scexpr RegAddress address_z = 0x08;

        uint8_t :4;
        uint8_t data:4;
    };

    struct TempReg:public Reg8<>{
        scexpr RegAddress address = 0x09;
        uint8_t :8;
        operator int() const {
            return ((uint8_t(*this) * int(0.8 * 65536) >> 16) - 75);
        }
    };

    struct Status1Reg:public Reg8<>{
        scexpr RegAddress address = 0x18;

        uint8_t:4;

        uint8_t otp_read_done:1;
        uint8_t selftest_sig:1;
        uint8_t mag_measure_done:1;
        uint8_t temp_measure_done:1;
    };

    struct OdrReg:public Reg8<>{
        using Reg8::operator=;

        scexpr RegAddress address = 0x1a;
        DataRate data_rate;
    };

    struct Ctrl0Reg:public Reg8<>{
        scexpr RegAddress address = 0x1B;

        uint8_t do_mag_measure:1;
        uint8_t do_temp_measure:1;
        uint8_t :1;
        uint8_t do_set:1;

        uint8_t do_reset:1;
        uint8_t auto_sr_en:1;
        uint8_t auto_self_test_en:1;
        uint8_t cmm_freq_en:1;
    };

    struct Ctrl1Reg:public Reg8<>{
        scexpr RegAddress address = 0x1C;

        uint8_t bandwidth:2;
        uint8_t x_inhibit:1;
        uint8_t y_inhibit:1;
        uint8_t z_inhibit:1;

        uint8_t st_enp:1;
        uint8_t st_enm:1;
        uint8_t sw_reset:1;
    };

    struct Ctrl2Reg:public Reg8<>{
        scexpr RegAddress address = 0x1D;

        //These bits determine how many measurements are done before a set is executed, when the 
        // part is in continuous mode and the automatic set/reset is enabled. From 000 to 111, the sensor 
        // will do one set for every 1, 25, 75, 100, 250, 500, 1000, and 2000 samples. In order to enable 
        // this feature, both En_prd_set and Auto_SR must be set to 1, and the part should work in 
        // continuous mode. Please note that during this operation, the sensor will not be reset.

        uint8_t prd_set:3;
        uint8_t en_prd_set:1;
        uint8_t cont_en:1;
        uint8_t :2;
        uint8_t high_pwr:1;
    };

    struct AxisSelfTestXReg:public Reg8<>{
        using Reg8::operator=;
        scexpr RegAddress address = 0x1e;
        uint8_t :8;
    };

    struct AxisSelfTestYReg:public Reg8<>{
        using Reg8::operator=;
        scexpr RegAddress address = 0x1f;
        uint8_t :8;
    };

    struct AxisSelfTestZReg:public Reg8<>{
        using Reg8::operator=;
        scexpr RegAddress address = 0x20;
        uint8_t :8;
    };
    struct ProductIdReg:public Reg8<>{
        scexpr RegAddress address = 0x39;
        scexpr uint8_t correct_id = 0b00010000;
        
        uint8_t id;
    };

    AxisReg x_reg;
    AxisReg y_reg;
    AxisReg z_reg;
    ExtAxisReg x_ext_reg;
    ExtAxisReg y_ext_reg;
    ExtAxisReg z_ext_reg;
    TempReg temp_reg;
    Status1Reg stat_reg;

    OdrReg odr_reg;
    Ctrl0Reg ctrl0_reg;
    Ctrl1Reg ctrl1_reg;
    Ctrl2Reg ctrl2_reg;

    AxisSelfTestXReg x_st_reg;
    AxisSelfTestYReg y_st_reg;
    AxisSelfTestZReg z_st_reg;
    ProductIdReg product_id_reg;

};
class MMC5603:
    public MagnetometerIntf,
    public MMC5603_Regs{
public:

    MMC5603(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    MMC5603(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    MMC5603(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR)
        :i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<> set_data_rate(const DataRate dr);

    [[nodiscard]] IResult<> set_band_width(const BandWidth bw);

    [[nodiscard]] IResult<> enable_contious(const Enable en = EN);

    [[nodiscard]] IResult<> inhibit_channels(bool x, bool y, bool z);

    [[nodiscard]] IResult<Vec3<q24>> read_mag();

protected:

    hal::I2cDrv i2c_drv_;


    [[nodiscard]] IResult<> read_burst(const RegAddress addr, uint8_t * data, size_t len){
        if(const auto res = i2c_drv_.read_burst(uint8_t(addr), std::span(data, len));
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        const auto res = i2c_drv_.write_reg(
            uint8_t(reg.address), 
            reg.as_val(), LSB);
        if(res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
    
    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        const auto res = i2c_drv_.read_reg(
            uint8_t(reg.address), 
            reg.as_ref(), LSB);
        if(res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }


    [[nodiscard]] IResult<> set_self_test_threshlds(uint8_t x, uint8_t y, uint8_t z);

};

}