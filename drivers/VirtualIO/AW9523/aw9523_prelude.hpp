//这个驱动已经完成
//这个驱动已经测试

#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/gpio/vport.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

#include "primitive/pwm_channel.hpp"



namespace ymd::drivers{
struct AW9523_Prelude{
    enum class Error_Kind:uint8_t{
        WrongChipId,
        IndexOutOfRange
    };

    DEF_FRIEND_DERIVE_DEBUG(Error_Kind)
    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class CurrentLimit{
        Max, High, Medium, Low
    };

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b1011000);


    enum class RegAddr:uint8_t{
        In = 0x00,
        Out = 0x02,
        Dir = 0x04,
        Inten = 0x06,
        ChipId = 0x10,
        Ctl = 0x11,
        LedMode = 0x12,
        DimP10 = 0x20,
        DimP11 = 0x21,
        DimP12 = 0x22,
        DimP13 = 0x23,
        DimP00 = 0x24,
        DimP01 = 0x25,
        DimP02 = 0x26,
        DimP03 = 0x27,
        DimP04 = 0x28,
        DimP05 = 0x29,
        DimP06  = 0x2a,
        DimP07  = 0x2b,
        DimP14  = 0x2c,
        DimP15  = 0x2d,
        DimP16  = 0x2e,
        DimP17  = 0x2f,
        SwRst = 0x7f
    };

    struct Config{
        CurrentLimit current_limit = CurrentLimit::Low;
    };

    static constexpr uint8_t VALID_CHIP_ID = 0x23;
    static constexpr size_t MAX_CHANNELS = 16;
};

struct AW9523_Regset final:public AW9523_Prelude{

    
    struct InputReg  : public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::In;

        hal::PinMask mask = hal::PinMask::zero();
    }DEF_R16(input_reg)

    struct OutputReg  : public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::Out;

        hal::PinMask mask = hal::PinMask::zero();
    }DEF_R16(output_reg)

    struct DirReg:public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::Dir;

        hal::PinMask mask = hal::PinMask::zero();
    }DEF_R16(dir_reg)

    struct CtlReg:Reg8<>{
        static constexpr auto ADDRESS = RegAddr::Ctl;
        uint8_t isel:2;
        uint8_t __resv1__:2;
        uint8_t p0mod:1;
        uint8_t __resv2__:3;
    }DEF_R8(ctl_reg)

    struct IntEnReg:public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::Inten;

        hal::PinMask mask = hal::PinMask::zero();
    }DEF_R16(inten_reg)


    struct LedModeReg:public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::LedMode;

        hal::PinMask mask = hal::PinMask::zero();
    }DEF_R16(led_mode_reg);


    struct ChipIdReg:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::ChipId;

        uint8_t id;
    }DEF_R8(chip_id_reg)

    

};


};