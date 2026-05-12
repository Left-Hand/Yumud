#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/conn/i2c/i2cdrv.hpp"

namespace ymd::drivers{

struct [[nodiscard]] GD30AD3344_Prelude{
    enum class Error_Kind:uint8_t{
        ChipIdMismatch
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class [[nodiscard]] DataRate:uint8_t{
        _6_25Hz = 0,
        _12_5Hz, 
        _25Hz, 
        _50Hz, 
        _100Hz, 
        _250Hz, 
        _500Hz, 
        _1000Hz,
    };

    enum class [[nodiscard]] MUX:uint16_t{
        P0N1 = 0, P0N3, P1N3, P2N3, P0NG, P1NG, P2NG, P3NG
    };

    enum class [[nodiscard]] PGA:uint16_t{
        _2_3 = 0, _1, _2, _4, _8, _16
    };

};

struct [[nodiscard]] GD30AD3344_Regset:public GD30AD3344_Prelude{

/* Register 0x00 (CONVERSION) definition
* ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
* |  Bit 15  |  Bit 14  |  Bit 13  |  Bit 12  |  Bit 11  |  Bit 10  |   Bit 9  |   Bit 8  |   Bit 7  |   Bit 6  |   Bit 5  |   Bit 4  |   Bit 3  |   Bit 2  |   Bit 1  |   Bit 0  |
* ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
* |                                                                                    CONV[15:0]                                                                                   |
* ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/


/* Register 0x01 (CONFIG) definition
* ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
* |  Bit 15  |  Bit 14  |  Bit 13  |  Bit 12  |  Bit 11  |  Bit 10  |   Bit 9  |   Bit 8  |   Bit 7  |   Bit 6  |   Bit 5  |   Bit 4  |   Bit 3  |   Bit 2  |   Bit 1  |   Bit 0  |
* ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
* |    SS    |            MUX[2:0]            |            PGA[2:0]            |   MODE   |             DR[2:0]            | RESERVED |PULL_UP_EN|       NOP[1:0]      | RESERVED |
* ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
};


}

