#pragma once

#include <tuple>

#include "core/utils/reg_base.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "core/math/real.hpp"
#include "core/math/realmath.hpp"

#include "hal/conn/i2c/i2cdrv.hpp"


// https://blog.csdn.net/qq_42250136/article/details/148233168

namespace ymd::drivers{

struct TSL2591_Prelude{

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b1000100);


    enum class [[nodiscard]] Error_Kind:uint8_t{
        InvalidChipId
    };
    
    DEF_FRIEND_DERIVE_DEBUG(Error_Kind)
    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class [[nodiscard]] RegAddr:uint8_t{
        Enable = 0x00,
        Control = 0x01
    };

    struct [[nodiscard]] EnableCode final{
        // 内部振荡器总开关：1=芯片上电工作，0=芯片下电休眠
        uint8_t power_on:1;

        // ALS光强感应功能使能：1=开启，0=关闭 
        uint8_t aen:1;

        uint8_t __resv1__:2;

        // 置1时，开启ALS光强中断，触发规则受持久过滤器限制 
        uint8_t aien:1;

        uint8_t __resv2__:1;
        
        // 置1时，中断产生后，当前光强采样周期结束芯片自动进入休眠模式 
        uint8_t sai:1;

        // 置1时，光照超出中断阈值**立即触发中断**，忽略持久过滤器
        uint8_t npien:1;

        [[nodiscard]] constexpr uint8_t to_u8() const noexcept{
            return std::bit_cast<uint8_t>(*this);}
    };
};


struct TSL2591_Regset:public TSL2591_Prelude{



    struct R8_Enable:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr::Enable;

        EnableCode code;
    }DEF_R8(enable_reg)



    struct [[nodiscard]] Packet final{
        
    };
};


struct TSL2591 final:public TSL2591_Prelude{

};

}