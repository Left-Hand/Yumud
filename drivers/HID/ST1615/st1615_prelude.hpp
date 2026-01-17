#pragma once

// Apache-2.0 license
// https://github.com/embedded-drivers/sitronix-touch

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "algebra/vectors/vec2.hpp"

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

    struct [[nodiscard]] GestureId final{
        enum class [[nodiscard]] Kind:uint8_t{
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

        static constexpr IResult<GestureId> from_u8(const uint8_t bits){
            switch(bits){
                case NONE: return Ok(GestureId(None));
                case int(DoubleTab) ... int(Drag): 
                    return Ok(GestureId(std::bit_cast<Kind>(bits)));
                default:    return Err(Error::InvalidGestureId);
            }
        }

        [[nodiscard]] constexpr bool is_some() const { return kind_ != NONE; }
        [[nodiscard]] constexpr bool is_none() const { return kind_ == NONE; }

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
        static constexpr uint8_t NONE = 0x00;
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
            return os
                << os.field("max_touches")(self.max_touches) << os.splitter()
                << os.field("max_x")(self.max_x) << os.splitter()
                << os.field("max_y")(self.max_y)
            ;
        }
    };


    struct GestureInfo {
        GestureId gesture_id;
        bool proximity;
        bool water;

        friend OutputStream & operator<<(OutputStream & os, const GestureInfo & self){ 
            return os 
                << os.field("gesture_id")(self.gesture_id) << os.splitter()
                << os.field("proximity")(self.proximity) << os.splitter()
                << os.field("water")(self.water)
            ;
        }
    };

    using Point = Vec2<uint16_t>;
};

}
