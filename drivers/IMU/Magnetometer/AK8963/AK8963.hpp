#pragma once

#include "core/io/regs.hpp"
#include "drivers/IMU/IMU.hpp"
#include "drivers/IMU/details/AsahiKaseiIMU.hpp"

namespace ymd::drivers{

class AK8963:public MagnetometerIntf{
public:
    // "0000":  Power-down mode 
    // "0001":  Single measurement mode 
    // "0010":  Continuous measurement mode 1 
    // "0110":  Continuous measurement mode 2 
    // "0100":  External trigger measurement mode 
    // "1000": Self-test mode 
    // "1111": Fuse ROM access mode 

    enum class Mode:uint8_t{
        PowerDown = 0b0000,
        SingleMeasurement = 0b0001,
        ContMode1 = 0b0010,
        ContMode2 = 0b0110,
        ExtTrigger = 0b0100,
        SelfTest = 0b1000,
        FuseRomAccess = 0b1111,
    };

    using Error = ImuError;
protected:
    AsahiKaseiSensor_Phy phy_;

    // [[nodiscard]] virtual Result<void, Error> write_reg(const uint8_t addr, const uint8_t data);
    [[nodiscard]] Result<void, Error> write_reg(const uint8_t addr, const uint8_t data);

    // [[nodiscard]] virtual Result<void, Error> read_reg(const uint8_t addr, uint8_t & data);
    [[nodiscard]] Result<void, Error> read_reg(const uint8_t addr, uint8_t & data);

    // [[nodiscard]] virtual Result<void, Error> read_burst(const uint8_t reg_addr, int16_t * datas, const size_t len);
    [[nodiscard]] Result<void, Error> read_burst(const uint8_t reg_addr, int16_t * datas, const size_t len);

    using RegAddress = uint8_t;

    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0b00011000);

    struct R8_WIA:public Reg8<>{
        scexpr RegAddress address = 0x00;
        scexpr uint8_t correct = 0x48;

        uint8_t data;
    } wia_reg = {};

    struct R8_INFO:public Reg8<>{
        scexpr RegAddress address = 0x01;

        uint8_t data;
    } info_reg = {};

    struct R8_ST1:public Reg8<>{
        scexpr RegAddress address = 0x02;

        uint8_t drdy:1;
        uint8_t dor:1;
        uint8_t :6;

        bool is_data_ready() const {return drdy;}
        bool is_data_overrun() const {return dor;}
    } st1_reg = {};

    REG16I_QUICK_DEF(0x03, MagXReg, mag_x_reg);
    REG16I_QUICK_DEF(0x05, MagYReg, mag_y_reg);
    REG16I_QUICK_DEF(0x07, MagZReg, mag_z_reg);

    struct R8_ST2:public Reg8<>{
        scexpr RegAddress address = 0x09;

        uint8_t :3;
        uint8_t hofl:1;
        uint8_t bitm:1;
        uint8_t :3;
    } st2_reg = {};

    struct R8_CNTL1:public Reg8<>{
        scexpr RegAddress address = 0x0A;

        uint8_t mode:4;
        uint8_t bit:1;
        uint8_t :3;
    } cntl1_reg = {};

    struct R8_CNTL2:public Reg8<>{
        scexpr RegAddress address = 0x0B;

        uint8_t srst:1;
        uint8_t :7;
    } cntl2_reg = {};

    struct R8_ASTC:public Reg8<>{
        scexpr RegAddress address = 0x0C;

        uint8_t :6;
        uint8_t self:1;
        uint8_t :1;
    } astc_reg = {};

    struct R8_I2CDIS:public Reg8<>{
        scexpr RegAddress address = 0x0F;

        scexpr uint8_t key = 0b00011011;
        uint8_t data;
    } i2cdis_reg = {};

    struct _R8_ASA:public Reg8<>{
        uint8_t data;
    };

    struct R8_ASAX:public _R8_ASA{
        scexpr RegAddress address = 0x10;
    } asax_reg = {};

    struct R8_ASAY:public _R8_ASA{
        scexpr RegAddress address = 0x11;
    } asay_reg = {};

    struct R8_ASAZ:public _R8_ASA{
        scexpr RegAddress address = 0x12;
    } asaz_reg = {};

    bool data_valid_ = false;
    bool data_is_16_bits_ = false;

    Vector3_t<real_t> adj_scale;

    [[nodiscard]] Result<Vector3_t<uint8_t>, Error> get_coeff();

    static constexpr real_t conv_data_to_ut(const int16_t data, const bool is_16_bits){
        if(is_16_bits){
            return (data * iq_t<16>(0.15));
        }else{
            return (data * iq_t<16>(0.6));
        }
    }
public:

    AK8963(const hal::I2cDrv & i2c_drv):phy_(i2c_drv){;}
    AK8963(hal::I2cDrv && i2c_drv):phy_(i2c_drv){;}
    AK8963(hal::I2c & bus):phy_(hal::I2cDrv(bus, DEFAULT_I2C_ADDR)){;}
    AK8963(const hal::SpiDrv & spi_drv):phy_(spi_drv){;}
    AK8963(hal::SpiDrv && spi_drv):phy_(std::move(spi_drv)){;}
    AK8963(hal::Spi & spi, const hal::SpiSlaveIndex index):phy_(hal::SpiDrv(spi, index)){;}

    [[nodiscard]] Result<void, Error> init();
    [[nodiscard]] Result<void, Error> update();
    [[nodiscard]] Result<void, Error> validate();
    [[nodiscard]] Result<void, Error> reset();
    [[nodiscard]] Result<void, Error> busy();
    [[nodiscard]] Result<void, Error> stable();
    [[nodiscard]] Result<void, Error> disable_i2c();
    [[nodiscard]] Option<Vector3_t<real_t>> get_magnet();
    [[nodiscard]] Result<void, Error> set_data_width(const uint8_t bits);
    [[nodiscard]] Result<void, Error> set_mode(const Mode mode);
};
};