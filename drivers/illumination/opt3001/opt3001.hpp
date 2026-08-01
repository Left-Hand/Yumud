#pragma once

// OPT3001 是一款可如人眼般测量光强的单芯片照度
// 计。OPT3001 器件兼具精密的频谱响应和较强的 IR
// 阻隔功能，因此能够如人眼般准确测量光强且不受光源
// 影响。对于为追求美观效果而需要将传感器安装在深色
// 玻璃下的工业设计而言，较强的 IR 阻隔功能还有助于
// 保持高精度。OPT3001 专门针对构建基于光线的人眼
// 般体验的系统而设计，是人眼匹配度低且红外阻隔能力
// 差的光电二极管、光敏电阻或其它环境光传感器的首选
// 理想替代产品。

#include <tuple>

#include "core/utils/reg_base.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "core/math/real.hpp"

#include "hal/conn/i2c/i2cdrv.hpp"


namespace ymd::drivers{

struct OPT3001_Prelude{

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b1000100);


    enum class [[nodiscard]] Error_Kind:uint8_t{
        InvalidChipId
    };
    
    DEF_FRIEND_DERIVE_DEBUG(Error_Kind)
    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class RegAddr:uint8_t{
        RESULT = 0x00,
        CONFIG = 0x01,
        LOWLIMIT = 0x02,
        HIGHLIMIT = 0x03,
        MANUFACTUREID = 0x7E,
        DEVICEID = 0x7F,
    };


    struct [[nodiscard]] LightCode final{
        uint16_t bits;

        [[nodiscard]] constexpr uint32_t to_lux() const {
            const uint32_t result = bits & 0x0FFF;
            const uint32_t exponent = (bits >>12)&0x000F;
            int32_t right_shift_count = 6 - exponent;

            if(right_shift_count >= 0)
                return result >> right_shift_count;
            else
                return result << right_shift_count;
        }
    };
};


class OPT3001 final:
    public OPT3001_Prelude{
public:


    explicit OPT3001(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit OPT3001(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit OPT3001(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR):
        i2c_drv_(i2c, i2c_addr){;}

    OPT3001(const OPT3001 &) = delete;
    OPT3001(OPT3001 &&) = delete;
    ~OPT3001() = default;


    IResult<> init();


private:
    hal::I2cDrv i2c_drv_;

};


}