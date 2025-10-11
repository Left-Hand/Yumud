#pragma once

//这个驱动已经完成
//这个驱动还未测试

// https://github.com/embedded-drivers/sitronix-touch

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "types/vectors/vector2.hpp"
#include "core/magic/enum_traits.hpp"

namespace ymd::drivers{

struct ST1615_Prelude{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x55);
    static constexpr auto MAX_I2C_BAUDRATE = 18_KHz;
    static constexpr size_t MAX_POINTS_COUNT = 9;
    
    using RegAddr = uint8_t;

    static constexpr RegAddr STATUS = 0x01;
    static constexpr RegAddr CONTACT_COUNT_MAX = 0x3F;
    static constexpr RegAddr MISC_INFO = 0xF0;

    static constexpr RegAddr XY_RESOLUTION_H = 0x04;
    static constexpr RegAddr X_RESOLUTION_L = 0x05;
    static constexpr RegAddr Y_RESOLUTION_L = 0x06;

    static constexpr RegAddr SENSING_COUNTER_L = 0x07;
    static constexpr RegAddr SENSING_COUNTER_H = 0x08;

    static constexpr RegAddr ADVANCED_TOUCH_INFO = 0x10;


    enum class Error_Kind:uint8_t{
        PointNthOutOfRange,
        InvalidGestureId,
        RetryTimeout
    };

    DEF_FRIEND_DERIVE_DEBUG(Error_Kind)
    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

        struct GestureId{
        enum class Kind:uint8_t{
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

        DEF_FRIEND_DERIVE_DEBUG(Kind)

        using enum Kind;

        constexpr GestureId(Kind kind):kind_(std::bit_cast<uint8_t>(kind)) {;}
        constexpr GestureId(_None_t):kind_(0) {;}

        static constexpr IResult<GestureId> from_u8(const uint8_t raw){
            switch(raw){
                case k_None: return Ok(GestureId(None));
                case int(DoubleTab) ... int(Drag): 
                    return Ok(GestureId(std::bit_cast<Kind>(raw)));
                default:    return Err(Error::InvalidGestureId);
            }
        }

        constexpr bool is_some() const { return kind_ != k_None; }
        constexpr bool is_none() const { return kind_ == k_None; }

        constexpr Kind unwrap() const { 
            if(is_none()) __builtin_trap();
            return std::bit_cast<Kind>(kind_);
        }

        friend OutputStream & operator<<(OutputStream & os, GestureId id){
            if(id.is_some()){
                return os << id.unwrap();
            }else{
                return os << "None";
            }
        }
    private:
        static constexpr uint8_t k_None = 0x00;
        uint8_t kind_;
    };


    struct Capabilities {
        /// Maximum Number of Contacts Support Register
        uint8_t max_touches = 8;
        // XY resolution
        uint16_t max_x = 0xffff;
        uint16_t max_y = 0xffff;
        bool smart_wake_up = false;

        constexpr bool is_point_valid(const Vec2<uint16_t> point){
            return (point.x <= max_x) && (point.y <= max_y);
        }

        friend OutputStream & operator<<(OutputStream & os, const Capabilities & self){ 
            return os << os.scoped("Capabilities")(os 
                << os.field("max_touches")(os << self.max_touches) << os.splitter()
                << os.field("max_x")(os << self.max_x) << os.splitter()
                << os.field("max_y")(os << self.max_y)
            );
        }
    };


    struct GestureInfo {
        GestureId gesture_id;
        bool proximity;
        bool water;

        friend OutputStream & operator<<(OutputStream & os, const GestureInfo & self){ 
            return os << os.scoped("GestureId")(os 
                << os.field("gesture_id")(os << self.gesture_id) << os.splitter()
                << os.field("proximity")(os << self.proximity) << os.splitter()
                << os.field("water")(os << self.water)
            );
        }
    };

    using Point = Vec2<uint16_t>;
};

struct ST1615 final:public ST1615_Prelude{

    explicit ST1615(
        Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<GestureInfo> get_gesture_info();

    [[nodiscard]] IResult<Option<Point>> get_point(uint8_t nth);

    [[nodiscard]] IResult<uint16_t> get_sensor_count();

    [[nodiscard]] IResult<Capabilities> get_capabilities();

    [[nodiscard]] IResult<>  blocking_until_normal_status();

private:
    hal::I2cDrv i2c_drv_;
    Capabilities capabilities_;

    [[nodiscard]] IResult<uint8_t> read_reg8(uint8_t reg){
        std::array<uint8_t, 2>buf = {0, 1};
        if(const auto res = i2c_drv_.read_burst<uint8_t>(reg, buf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok(buf[0]);
    }

    [[nodiscard]] IResult<> read_burst(uint8_t reg, std::span<uint8_t> buf){ 
        if(const auto res = i2c_drv_.read_burst<uint8_t>(reg, buf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

};

}
