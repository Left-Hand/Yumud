
#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/conn/spi/spidrv.hpp"
#include "middlewares/algebra/vectors/vec2.hpp"

// https://github.com/ttwards/motor/blob/939f1db78dcaae6eb819dcb54b6146d94db7dffc/drivers/sensor/paw3395/paw3395.h#L122
// https://github.com/dotdotchan/bs2x_sdk/blob/main/application/samples/products/sle_mouse_with_dongle/mouse_sensor/mouse_sensor_paw3395.c

namespace ymd::drivers::paw3395{


struct PAW3395_Prelude{
    /* PAW3395 Product ID value */
    static constexpr  uint8_t PAW3395_PRODUCT_ID = 0x51; /* Verify this value with datasheet */

    enum class Error_Kind:uint8_t{
        InvalidProductId,
        InitTimeout,
        DataNotReady
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class [[nodiscard]] RegAddr:uint8_t{

        // 产品ID，RO，Default：0x51
        PRODUCT_ID = 0x00,   

        // 修订版本ID，RO，Default：0x00
        REVISION_ID = 0x01,   

        // 运动状态，R/W，Default：0x00
        MOTION = 0x02,   

        // X轴低位增量，RO，Default：0x00
        DELTA_X_L = 0x03,   

        // X轴高位增量，RO，Default：0x00
        DELTA_X_H = 0x04,   

        // Y轴低位增量，RO，Default：0x00
        DELTA_Y_L = 0x05,   

        // Y轴高位增量，RO，Default：0x00
        DELTA_Y_H = 0x06,   

        // 表面质量指标，RO，Default：0x00
        SQUAL = 0x07,   

        // 原始数据和，RO，Default：0x00
        RAWDATA_SUM = 0x08,   

        // 最大原始数据，RO，Default：0x00
        MAXIMUM_RAWDATA = 0x09,   

        // 最小原始数据，RO，Default：0x00
        MINIMUM_RAWDATA = 0x0A,   

        // 快门下限，RO，Default：0x00
        SHUTTER_LOWER = 0x0B,   

        // 快门上限，RO，Default：0x01
        SHUTTER_UPPER = 0x0C,   

        // 观测值，R/W，Default：0x80
        OBSERVATION = 0x15,   

        // 运动突发，R/W，Default：0x00
        MOTION_BURST = 0x16,   

        // 上电复位，WO，No default
        POWER_UP_RESET = 0x3A,   

        // 关闭，WO，No default
        SHUTDOWN = 0x3B,   

        // 性能配置，R/W，Default：0x00
        PERFORMANCE = 0x40,   

        // 设置分辨率(更新CPI)，WO，Default：0x00
        SET_RESOLUTION = 0x47,   

        // X轴低分辨率，R/W，Default：0x63
        RESOLUTION_X_LOW = 0x48,   

        // X轴高分辨率，R/W，Default：0x00
        RESOLUTION_X_HIGH = 0x49,   

        // Y轴低分辨率，R/W，Default：0x63
        RESOLUTION_Y_LOW = 0x4A,   

        // Y轴高分辨率，R/W，Default：0x00
        RESOLUTION_Y_HIGH = 0x4B,   

        // 角度捕捉，R/W，Default：0x0D
        ANGLE_SNAP = 0x56,   

        // 原始数据输出，RO，Default：0x00
        RAWDATA_OUTPUT = 0x58,   

        // 原始数据状态，RO，Default：0x00
        RAWDATA_STATUS = 0x59,   

        // 纹波控制，R/W，Default：0x00
        RIPPLE_CONTROL = 0x5A,   

        // 坐标系翻转，R/W，Default：0x60
        AXIS_CONTROL = 0x5B,   

        // 运动控制，R/W，Default：0x02
        MOTION_CTRL = 0x5C,   

        // 反向产品ID，RO，Default：0xAE
        INV_PRODUCT_ID = 0x5F,   

        // 运行下移，R/W，Default：0x14
        RUN_DOWNSHIFT = 0x77,   

        // 休息1周期，R/W，Default：0x01
        REST1_PERIOD = 0x78,   

        // 休息1下移，R/W，Default：0x90
        REST1_DOWNSHIFT = 0x79,   

        // 休息2周期，R/W，Default：0x19
        REST2_PERIOD = 0x7A,   

        // 休息2下移，R/W，Default：0x5E
        REST2_DOWNSHIFT = 0x7B,   

        // 休息3周期，R/W，Default：0x3F
        REST3_PERIOD = 0x7C,   

        // 运行下移倍数，R/W，Default：0x07
        RUN_DOWNSHIFT_MULT = 0x7D,   

        // 休息下移倍数，R/W，Default：0x55
        REST_DOWNSHIFT_MULT = 0x7E,   

    };
    // ANGLE_TUNE1 = 0x0577, // 角度调整1，R/W，Default：0x00
    // ANGLE_TUNE2 = 0x0578, // 角度调整2使能，R/W，Default：0x00
    // LIFT_CONFIG = 0x0C4E, // 抬起配置，R/W，Default：0x08




};
struct PAW3395_Regs:public PAW3395_Prelude{

};
}