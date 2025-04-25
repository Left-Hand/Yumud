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

    enum class Error:uint8_t{
        I2C_NOT_RESPONED = 0x00,
        I2C_NOT_ACK = 0x01,
        I2C_NOT_READY = 0x02,
        I2C_NOT_RECEIVED = 0x03,
        I2C_NOT_SENT = 0x04,
        I2C_NOT_TRANSFERED = 0x05,
        I2C_NOT_TRANSFERED_BYTE = 0x06,
        I2C_NOT_TRANSFERED_BYTE_ACK = 0x07,
        
        DEVICE_NOT_FOUNDED = 0x10,
        DEVICE_WHOAMI_FAILED,
        SENS_OVERFLOW,
        DATA_NOT_READY,

        UNSPECIFIED = 0xff
    };


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

    Result<Vector3_t<uint8_t>, Error> getCoeff();

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

    Result<void, Error> init();
    void update();
    Result<void, Error> verify();
    Result<void, Error> reset();
    Result<void, Error> busy();
    Result<void, Error> stable();
    Result<void, Error> disableI2c();
    Option<Vector3_t<real_t>> get_magnet();
    Result<void, Error> set_data_width(const uint8_t bits);
    Result<void, Error> setMode(const Mode mode);
};
};

namespace ymd::custom{
    template<typename T>
    struct result_converter<T, drivers::AK8963::Error, hal::BusError> {
        static Result<T, drivers::AK8963::Error> convert(const hal::BusError berr){
            using Error = drivers::AK8963::Error;
            
            if(berr.ok()) return Ok();

            Error err = [](const hal::BusError berr_){
                switch(berr_.type){
                    case hal::BusError::NO_ACK : return Error::I2C_NOT_ACK;

                    // case hal::BusError::I2C_NOT_READY: return AK8963::Error::I2C_NOT_READY;
                    default: return Error::UNSPECIFIED;
                }
            }(berr);

            return Err(err); 
        }
    };
}