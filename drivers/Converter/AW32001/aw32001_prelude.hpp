#pragma once



#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
namespace ymd::drivers{

struct AW32001_Prelude{
    enum class ErrorKind:uint8_t{
        NotReady,
        UnexpectedProductId,
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, ErrorKind)

    template<typename T = void>
    using IResult = Result<Error, T>;

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x34);

    using RegAddr = uint8_t;
};


struct AW32001_Regs:public AW32001_Prelude{ 
    enum class VIN_DPM : uint8_t {
        _3_88V = 0b0000,
        _3_96V = 0b0001,
        _4_04V = 0b0010,
        _4_12V = 0b0011,
        _4_20V = 0b0100,
        _4_28V = 0b0101,
        _4_36V = 0b0110,
        _4_44V = 0b0111,
        _4_52V = 0b1000,
        _4_60V = 0b1001,
        _4_68V = 0b1010,
        _4_76V = 0b1011,
        _4_84V = 0b1100,
        _4_92V = 0b1101,
        _5_00V = 0b1110,
        _5_08V = 0b1111
    };


    enum class IIN_LIM:uint8_t{
        _50mA = 0b0000,
        _80mA = 0b0001,
        //to 500ma
        
    }
};


}