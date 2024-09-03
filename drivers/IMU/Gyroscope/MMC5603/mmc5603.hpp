#pragma once

#include "../drivers/IMU/IMU.hpp"
#include "../hal/bus/i2c/i2cdrv.hpp"
#include <tuple>

#pragma GCC diagnostic ignored "-Wstrict-aliasing"

// #define MMC5603_DEBUG

#ifdef MMC5603_DEBUG
#undef MMC5603_DEBUG
#define MMC5603_DEBUG(...) DEBUG_PRINTLN(SpecToken::Space, std::hex, ##__VA_ARGS__, "\t|", __PRETTY_FUNCTION__);
#else
#define MMC5603_DEBUG(...)
#endif

class MMC5603:public Magnetometer{
public:
    static constexpr uint8_t default_i2c_addr = 0b01100000;

    MMC5603(const I2cDrv & _bus_drv):i2c_drv(_bus_drv){;}
    MMC5603(I2cDrv && _bus_drv):i2c_drv(_bus_drv){;}
    MMC5603(I2c & i2c, const uint8_t addr = default_i2c_addr):i2c_drv(I2cDrv(i2c, addr)){;}

    void update();

    bool verify();

    void reset();

    void setDataRate(const uint dr);

    void enableContious(const bool en = true);

    std::tuple<real_t, real_t, real_t> getMagnet() override;

protected:
    using RegAddress = uint8_t;
    struct AxisReg{
        static constexpr uint8_t address_x = 0x00;
        static constexpr uint8_t address_y = 0x02;
        static constexpr uint8_t address_z = 0x04;

        uint8_t data_h;
        uint8_t data_l;
    };

    struct ExtAxisReg{
        static constexpr uint8_t address_x = 0x06;
        static constexpr uint8_t address_y = 0x07;
        static constexpr uint8_t address_z = 0x08;

        uint8_t :4;
        uint8_t data:4;
    };

    struct TempReg{
        static constexpr uint8_t address = 0x09;
        uint8_t data;
        operator int() const {
            return ((data * int(0.8 * 65536) >> 16) - 75);
        }
    };

    struct Status1Reg{
        static constexpr uint8_t address = 0x18;

        uint8_t:4;

        uint8_t otp_read_done:1;
        uint8_t selftest_sig:1;
        uint8_t mag_measure_done:1;
        uint8_t temp_measure_done:1;
    };

    enum class DataRate:uint8_t{

    };

    struct OdrReg{
        static constexpr uint8_t address = 0x1a;
        uint8_t data;
    };


    struct Ctrl0Reg{
        static constexpr uint8_t address = 0x1B;

        uint8_t do_mag_measure:1;
        uint8_t do_temp_measure:1;
        uint8_t :1;
        uint8_t do_set:1;

        uint8_t do_reset:1;
        uint8_t auto_sr_en:1;
        uint8_t auto_self_test_en:1;
        uint8_t cmm_freq_en:1;
    };

    enum class BandWidth:uint8_t{
        _6_6ms,
        _3_5ms,
        _2_0ms,
        _1_2ms
    };

    struct Ctrl1Reg{
        static constexpr uint8_t address = 0x1C;

        uint8_t bandwidth:2;
        uint8_t x_inhibit:1;
        uint8_t y_inhibit:1;
        uint8_t z_inhibit:1;

        uint8_t st_enp:1;
        uint8_t st_enm:1;
        uint8_t sw_reset:1;
    };

    struct Ctrl2Reg{
        static constexpr uint8_t address = 0x1D;

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


    struct AxisSelfTestReg{
        static constexpr uint8_t address_x = 0x1e;
        static constexpr uint8_t address_y = 0x1f;
        static constexpr uint8_t address_z = 0x20;

        uint8_t data;
    };

    struct ProductIdReg{
        static constexpr uint8_t address = 0x39;
        static constexpr uint8_t correct_id = 0b00010000;

        uint8_t id;

    };

    struct {
        AxisReg x_reg;
        AxisReg y_reg;
        AxisReg z_reg;
        ExtAxisReg x_ext_reg;
        ExtAxisReg y_ext_reg;
        ExtAxisReg z_ext_reg;
        TempReg temp_reg;
        Status1Reg stat_reg;
        Ctrl0Reg ctrl0_reg;
        Ctrl1Reg ctrl1_reg;
        Ctrl2Reg ctrl2_reg;

        AxisSelfTestReg x_st_reg;
        AxisSelfTestReg y_st_reg;
        AxisSelfTestReg z_st_reg;
        ProductIdReg product_id_reg;
    };

    I2cDrv i2c_drv;

    void writeReg(const RegAddress address, const uint8_t reg){
        i2c_drv.writeReg((uint8_t)address, reg);
    }

    void readReg(const RegAddress address, uint8_t & reg){
        i2c_drv.readReg((uint8_t)address, reg);
    }

    void requestPool(const RegAddress addr, uint8_t * data, size_t len){
        i2c_drv.readPool((uint8_t)addr, data, len, MSB);
    }
public:

};