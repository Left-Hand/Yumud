#pragma once

#include "drivers/device_defs.h"
#include "drivers/IMU/IMU.hpp"

#include "sys/utils/Option.hpp"
#include "sys/utils/Result.hpp"



namespace ymd::drivers{
class AK8963:public Magnetometer{
public:
    enum class Mode:uint8_t{
        PowerDown = 0b0000,
        SingleMeasurement = 0b0001,
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
        
        DATA_NOT_READY = 0x10,

        UNSPECIFIED = 0xff
    };


protected:
    std::optional<hal::I2cDrv> i2c_drv_;
    std::optional<hal::SpiDrv> spi_drv_;

    // [[nodiscard]] virtual Result<void, Error> writeReg(const uint8_t addr, const uint8_t data);
    [[nodiscard]] Result<void, Error> writeReg(const uint8_t addr, const uint8_t data);

    // [[nodiscard]] virtual Result<void, Error> readReg(const uint8_t addr, uint8_t & data);
    [[nodiscard]] Result<void, Error> readReg(const uint8_t addr, uint8_t & data);

    // [[nodiscard]] virtual Result<void, Error> requestData(const uint8_t reg_addr, int16_t * datas, const size_t len);
    [[nodiscard]] Result<void, Error> requestData(const uint8_t reg_addr, int16_t * datas, const size_t len);

    using RegAddress = uint8_t;

    scexpr uint8_t default_i2c_addr = 0b00011000;

    struct{
        int16_t x;
        int16_t y;
        int16_t z;

        uint8_t x_adj;
        uint8_t y_adj;
        uint8_t z_adj;
    };

    struct R8_WIA:public Reg8<>{
        scexpr RegAddress address = 0x1b;
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
    REG16I_QUICK_DEF(0x06, MagZReg, mag_z_reg);

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
        scexpr RegAddress address = 0x0B;

        uint8_t :6;
        uint8_t self:1;
        uint8_t :1;
    } astc_reg = {};

    struct R8_I2CDIS:public Reg8<>{
        scexpr RegAddress address = 0x0B;

        scexpr uint8_t key = 0b00011011;
        uint8_t data;
    } i2cdis_reg = {};

    struct _R16_ASA:public Reg16i<>{
        uint8_t data;
    };

    struct R16_ASAX:public _R16_ASA{
        scexpr RegAddress address = 0x10;
    } asax_reg = {};

    struct R16_ASAY:public _R16_ASA{
        scexpr RegAddress address = 0x11;
    } asay_reg = {};

    struct R16_ASAZ:public _R16_ASA{
        scexpr RegAddress address = 0x12;
    } asaz_reg = {};

public:

    AK8963(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    AK8963(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    AK8963(hal::I2c & bus):i2c_drv_(hal::I2cDrv(bus, default_i2c_addr)){;}
    AK8963(const hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    AK8963(hal::SpiDrv && spi_drv):spi_drv_(std::move(spi_drv)){;}
    AK8963(hal::Spi & spi, const uint8_t index):spi_drv_(hal::SpiDrv(spi, index)){;}

    Result<void, Error> init();
    void update();
    Result<void, Error> verify();
    Result<void, Error> reset();
    Result<void, Error> busy();
    Result<void, Error> stable();
    Result<void, Error> setMode(const Mode mode);
    Result<void, Error> disableI2c();
    std::tuple<real_t, real_t, real_t> getMagnet();
};
};

namespace ymd::custom{
    template<typename T>
    struct result_converter<T, drivers::AK8963::Error, BusError> {
        static Result<T, drivers::AK8963::Error> convert(const BusError berr){
            using Error = drivers::AK8963::Error;
            using BusError = BusError;
            
            if(berr.ok()) return Ok();

            Error err = [](const BusError berr_){
                switch(berr_.type){
                    case BusError::NO_ACK : return Error::I2C_NOT_ACK;

                    // case BusError::I2C_NOT_READY: return AK8963::Error::I2C_NOT_READY;
                    default: return Error::UNSPECIFIED;
                }
            }(berr);

            return Err(err); 
        }
    };
}