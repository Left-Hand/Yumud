//这个驱动已经完成
//这个驱动还未测试

#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/gpio/vport.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{ 


struct NCA9555_Collections{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0b01000000);

    using RegAddress = uint8_t;

    enum class Error_Kind{
        IndexOutOfRange
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;
};

struct NCA9555_Regs:public NCA9555_Collections{
    struct InputReg:public Reg16<>{
        static constexpr RegAddress address = 0x00;
        hal::PinMask mask;
    };

    struct OutputReg:public Reg16<>{
        static constexpr RegAddress address = 0x02;
        hal::PinMask mask;
    };

    struct InversionReg:public Reg16<>{
        static constexpr RegAddress address = 0x04;
        hal::PinMask mask;
    };

    struct ConfigReg:public Reg16<>{
        static constexpr RegAddress address = 0x06;
        hal::PinMask mask;
    };

    InputReg input_reg = {};
    OutputReg output_reg = {};
    InversionReg inversion_reg = {};
    ConfigReg config_reg = {};
};

class NCA9555 final:public NCA9555_Regs{
public:

    NCA9555(hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    NCA9555(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    NCA9555(
        hal::I2c & i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_(hal::I2cDrv{i2c, DEFAULT_I2C_ADDR}){;}

    IResult<> init();
    IResult<> set_inversion(const hal::PinMask mask);
    IResult<> write_port(const uint16_t data);
    IResult<uint16_t> read_port();
    IResult<> set_mode(const size_t index, const hal::GpioMode mode);
private:
    hal::I2cDrv i2c_drv_;


    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = i2c_drv_.write_reg(uint8_t(reg.address), reg.as_val(), LSB);
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
    
    template<typename T>
    IResult<> read_reg(T & reg){
        if(const auto res = i2c_drv_.read_reg(uint8_t(reg.address), reg.as_ref(), LSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    
};
}