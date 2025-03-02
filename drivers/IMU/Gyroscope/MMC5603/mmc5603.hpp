#pragma once

#include "drivers/device_defs.h"
#include "drivers/IMU/IMU.hpp"


// #define MMC5603_DEBUG

#ifdef MMC5603_DEBUG
#undef MMC5603_DEBUG
#define MMC5603_DEBUG(...) DEBUG_PRINTLN(SpecToken::Space, std::hex, ##__VA_ARGS__, "\t|", __PRETTY_FUNCTION__);
#else
#define MMC5603_DEBUG(...)
#endif

namespace ymd::drivers{

class MMC5603:public Magnetometer{
public:
    scexpr uint8_t default_i2c_addr = 0b01100000;


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
    
    MMC5603(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    MMC5603(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    MMC5603(hal::I2c & i2c, const uint8_t addr = default_i2c_addr):i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    void update();

    bool verify();

    void reset();

    void setDataRate(const DataRate dr);

    void setBandWidth(const BandWidth bw);

    void enableContious(const bool en = true);

    void inhibitChannels(bool x, bool y, bool z);

    std::tuple<real_t, real_t, real_t> getMagnet() override;

protected:
    using RegAddress = uint8_t;

    struct AxisReg:public Reg16{
        scexpr RegAddress address_x = 0x00;
        scexpr RegAddress address_y = 0x02;
        scexpr RegAddress address_z = 0x04;

        uint8_t data_h;
        uint8_t data_l;
    };

    struct ExtAxisReg:public Reg8{
        scexpr RegAddress address_x = 0x06;
        scexpr RegAddress address_y = 0x07;
        scexpr RegAddress address_z = 0x08;

        uint8_t :4;
        uint8_t data:4;
    };

    struct TempReg:public Reg8{
        scexpr RegAddress address = 0x09;
        uint8_t :8;
        operator int() const {
            return ((uint8_t(*this) * int(0.8 * 65536) >> 16) - 75);
        }
    };

    struct Status1Reg:public Reg8{
        scexpr RegAddress address = 0x18;

        uint8_t:4;

        uint8_t otp_read_done:1;
        uint8_t selftest_sig:1;
        uint8_t mag_measure_done:1;
        uint8_t temp_measure_done:1;
    };

    struct OdrReg:public Reg8{
        using Reg8::operator=;

        scexpr RegAddress address = 0x1a;
        uint8_t :8;
    };

    struct Ctrl0Reg:public Reg8{
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

    struct Ctrl1Reg:public Reg8{
        scexpr RegAddress address = 0x1C;

        uint8_t bandwidth:2;
        uint8_t x_inhibit:1;
        uint8_t y_inhibit:1;
        uint8_t z_inhibit:1;

        uint8_t st_enp:1;
        uint8_t st_enm:1;
        uint8_t sw_reset:1;
    };

    struct Ctrl2Reg:public Reg8{
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

    struct AxisSelfTestXReg:public Reg8{
        using Reg8::operator=;
        scexpr RegAddress address = 0x1e;
        uint8_t :8;
    };

    struct AxisSelfTestYReg:public Reg8{
        using Reg8::operator=;
        scexpr RegAddress address = 0x1f;
        uint8_t :8;
    };

    struct AxisSelfTestZReg:public Reg8{
        using Reg8::operator=;
        scexpr RegAddress address = 0x20;
        uint8_t :8;
    };
    struct ProductIdReg:public Reg8{
        scexpr RegAddress address = 0x39;
        scexpr uint8_t correct_id = 0b00010000;
        
        uint8_t :8;
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

    hal::I2cDrv i2c_drv_;

    void writeReg(const RegAddress address, const uint8_t reg){
        i2c_drv_.writeReg((uint8_t)address, reg);
    }

    void readReg(const RegAddress address, uint8_t & reg){
        i2c_drv_.readReg((uint8_t)address, reg);
    }

    void requestPool(const RegAddress addr, uint8_t * data, size_t len){
        i2c_drv_.readMulti((uint8_t)addr, data, len);
    }

    void setSelfTestThreshlds(uint8_t x, uint8_t y, uint8_t z);

};

}