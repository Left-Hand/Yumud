#pragma once

//这个驱动已经完成
//这个驱动还未测试

// https://github.com/embedded-drivers/sitronix-touch

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"


namespace ymd::drivers{

struct ST1615_Prelude{
    static constexpr const auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x55);
    using RegAddress = uint8_t;

    static constexpr RegAddress STATUS = 0x01;
    static constexpr RegAddress CONTACT_COUNT_MAX = 0x3F;
    static constexpr RegAddress MISC_INFO = 0xF0;

    static constexpr RegAddress XY_RESOLUTION_H = 0x04;
    static constexpr RegAddress X_RESOLUTION_L = 0x05;
    static constexpr RegAddress Y_RESOLUTION_L = 0x06;

    static constexpr RegAddress SENSING_COUNTER_L = 0x07;
    static constexpr RegAddress SENSING_COUNTER_H = 0x08;

    static constexpr RegAddress ADVANCED_TOUCH_INFO = 0x10;

    static constexpr size_t MAX_BLOCKING_TIMES = 20;


    enum class Error_Kind:uint8_t{
        PointRankOutOfRange,
        RetryTimeout
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class GestureType:uint8_t{
        None = 0,
        DoubleTab = 1,
        ZoomIn = 2,
        ZoomOut = 3,
        SlideLeftToRight = 4,
        SlideRightToLeft = 5,
        SlideTopToBottom = 6,
        SlideBottomToTop = 7,
        Palm = 8,
        SingleTap = 9,
        LongPress = 10,
        EndOfLongPress = 11,
        Drag = 12,
    };

    struct Capabilities {
        /// Maximum Number of Contacts Support Register
        uint8_t max_touches;
        // XY resolution
        uint16_t max_x;
        uint16_t max_y;
        bool smart_wake_up;
    };


    struct GestureInfo {
        GestureType gesture_type;
        bool proximity;
        bool water;
    };

    struct Point{
        uint16_t x;
        uint16_t y;
    };
};

struct ST1615 final:public ST1615_Prelude{

    ST1615(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c.deref(), addr)){;}

    IResult<> init();

    IResult<GestureInfo> get_gesture_info();

    IResult<Option<Point>> get_point(uint8_t nth);

    IResult<uint16_t> get_sensor_count();

    IResult<Capabilities> get_capabilities();

    IResult<>  blocking_until_normal_status();

private:
    hal::I2cDrv i2c_drv_;

    IResult<uint8_t> read_reg8(uint8_t reg){
        std::array<uint8_t, 2>buf = {0, 1};
        if(const auto res = i2c_drv_.read_burst<uint8_t>(reg, buf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok(buf[0]);
    }

    IResult<> read_u8(uint8_t reg, std::span<uint8_t> buf){ 
        if(const auto res = i2c_drv_.read_burst<uint8_t>(reg, buf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

};

}
