//这个驱动已经完成
//这个驱动还未测试

#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/gpio/vport.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{ 


struct NCA9555_Prelude{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b01000000 >> 1);

    using RegAddr = uint8_t;

    enum class Error_Kind{
        IndexOutOfRange
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;
};

struct NCA9555_Regs:public NCA9555_Prelude{
    struct R16_Input:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x00;
        hal::PinMask mask = hal::PinMask::zero();
    };

    struct R16_Output:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x02;
        hal::PinMask mask = hal::PinMask::zero();
    };

    struct R16_Inversion:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x04;
        hal::PinMask mask = hal::PinMask::zero();
    };

    struct R16_Config:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x06;
        hal::PinMask mask = hal::PinMask::zero();
    };

    R16_Input input_reg = {};
    R16_Output output_reg = {};
    R16_Inversion inversion_reg = {};
    R16_Config config_reg = {};
};

class NCA9555 final:public NCA9555_Regs{
public:

    explicit NCA9555(hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit NCA9555(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit NCA9555(
        Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_(hal::I2cDrv{i2c, DEFAULT_I2C_ADDR}){;}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> set_inversion(const hal::PinMask mask);
    [[nodiscard]] IResult<> write_port(const uint16_t data);
    [[nodiscard]] IResult<uint16_t> read_port();
    [[nodiscard]] IResult<> set_mode(const Nth nth, const hal::GpioMode mode);
private:
    hal::I2cDrv i2c_drv_;


    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = i2c_drv_.write_reg(std::bit_cast<uint8_t>(T::ADDRESS), reg.to_bits(), std::endian::little);
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
    
    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        if(const auto res = i2c_drv_.read_reg(std::bit_cast<uint8_t>(T::ADDRESS), reg.as_bits_mut(), std::endian::little);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    
};
}