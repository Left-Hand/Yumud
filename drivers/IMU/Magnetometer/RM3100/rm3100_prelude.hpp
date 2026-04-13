// 参考来源
// https://github.com/hnguy169/RM3100-Arduino
// https://github.com/lmnop100/RM3100-Driver/blob/master/RM3100.h
// https://github.com/bllovetx/rm3100/blob/master/src/lib.rs

//  * 注意：本实现为完全原创，未使用上述项目的任何代码。
//  * 参考仅用于理解问题领域，未复制任何具体实现。

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/conn/i2c/i2cdrv.hpp"

#include "algebra/vectors/vec3.hpp"


namespace ymd::drivers{

struct RM3100_Prelude{


static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x20);

enum class [[nodiscard]] RegAddr:uint8_t{
    // regs
    POLL = 0x00,
    CMM = 0x01,
    CCX1        = 0x04, 
    CCX0        = 0x05, 
    CCY1        = 0x04, 
    CCY0        = 0x05,  
    CCZ1        = 0x04, 
    CCZ0        = 0x05,  
    NOS			= 0x0A,
    TMRC = 0x0B,
    MX = 0x24,
    MY = 0x27,
    MZ = 0x2A,
    BIST = 0x33,
    STATUS = 0x34,
    HSHAKE = 0x35,
    REVID = 0x36,

    MX2   		= 0xA4,
    MX1   		= 0xA5,
    MX0   		= 0xA6,
    MY2   		= 0xA7,
    MY1   		= 0xA8,
    MY0   		= 0xA9,
    MZ2   		= 0xAA,
    MZ1   		= 0xAB,
    MZ0   		= 0xAC,
};

//   gain = (0.3671 * (float)cycleCount) + 1.5; //linear equation to calculate the gain from cycle count

enum class [[nodiscard]] Error_Kind:uint8_t{
    ChipIdMismatch,
    IndexOutOfRange
};

enum class [[nodiscard]] UpdateRate:uint8_t {
    _600Hz = 0x92,
    _300Hz = 0x93,
    _150Hz = 0x94,
    _75Hz = 0x95,
    _37Hz = 0x96,
    _18Hz = 0x97,
    _9Hz = 0x98,
    _4_5Hz = 0x99,
    _2_3Hz = 0x9A,
    _1_2Hz = 0x9B,
    _0_6Hz = 0x9C,
    _0_3Hz = 0x9D,
    _0_15Hz = 0x9E,
    _0_075Hz = 0x9F,
};

enum class [[nodiscard]] DRDM:uint8_t {
    AlarmFull = 0b0000,
    Any = 0b0100,
    Full = 0b1000,
    Alarm = 0b1100,
};


DEF_FRIEND_DERIVE_DEBUG(Error_Kind)
DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

template<typename T = void>
using IResult = Result<T, Error>;

};


}