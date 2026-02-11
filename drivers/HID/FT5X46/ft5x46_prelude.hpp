#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

#include "algebra/vectors/vec2.hpp"


namespace ymd::drivers::ft5x46{

struct [[nodiscard]] FT5X46_Prelude{

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x38);
    static constexpr auto MAX_I2C_BAUDRATE = 200000;
    static constexpr size_t NUM_MAX_TOUCH_POINTS = 5;
    static constexpr size_t REGISTER_LEN = 1;


    using RegAddr = uint8_t;

    enum class [[nodiscard]] ErrorKind:uint8_t{
        Unspecified,
        InvalidGestureId,
        NthGreatThan1
    };

    DEF_FRIEND_DERIVE_DEBUG(ErrorKind)
    DEF_ERROR_SUMWITH_HALERROR(Error, ErrorKind)

    template<typename T = void>
    using IResult = Result<T, Error>;
        
    enum class EventFlag : uint8_t {
        PressDown = 0x0,
        LiftUp = 0x1,
        Contact = 0x2,
        None = 0x3
    };

    enum class GestureId : uint8_t {
        MoveUp = 0x10,
        MoveRight = 0x14,
        MoveDown = 0x18,
        MoveLeft = 0x1C,
        MoveIn = 0x48,
        MoveOut = 0x49,
        None = 0x00
    };

    struct [[nodiscard]] Point{
        uint16_t x;
        uint16_t y;
        uint16_t weight;

        static constexpr Point zero(){
            return Point{0, 0, 0};
        }

        static constexpr Point from_default(){
            return zero();
        }
    };
};


struct [[nodiscard]] FT5X46_Regs : public FT5X46_Prelude {
    
    struct [[nodiscard]] R8_PxH : public Reg8<> {
        static constexpr RegAddr ADDRESS = 0x03;

        
        uint8_t x_high : 4;
        uint8_t : 2;
        EventFlag event_flag : 2;
    } DEF_R8(pxh)

    struct [[nodiscard]] R8_PxL : public Reg8<> {
        static constexpr RegAddr ADDRESS = 0x04;
        uint8_t x_low;
    } DEF_R8(pxl)

    struct [[nodiscard]] R8_PyH : public Reg8<> {
        static constexpr RegAddr ADDRESS = 0x05;
        
        uint8_t y_high : 4;
        uint8_t touch_id : 4;  // Touch ID of Touch Point, this value is 0x0F when the ID is invalid
    } DEF_R8(pyh)

    struct [[nodiscard]] R8_PyL : public Reg8<> {
        static constexpr RegAddr ADDRESS = 0x06;
        uint8_t y_low;
    } DEF_R8(pyl)

    struct [[nodiscard]] R8_PWeight : public Reg8<> {
        static constexpr RegAddr ADDRESS = 0x07;
        uint8_t weight;  // Touch pressure value
    } DEF_R8(pweight)

    struct [[nodiscard]] R8_PMisc : public Reg8<> {
        static constexpr RegAddr ADDRESS = 0x08;
        
        uint8_t : 4;
        uint8_t touch_area : 4;  // Touch area value
    } DEF_R8(pmisc)
};

struct [[nodiscard]] FT6336_MainRegs : public FT5X46_Prelude {
    
    struct [[nodiscard]] R8_ModeSwitch : public Reg8<> {
        static constexpr RegAddr ADDRESS = 0x00;
        
        enum class DeviceMode : uint8_t {
            Working = 0x0,
            Test = 0x4
        };
        
        uint8_t : 4;
        DeviceMode device_mode : 3;
        uint8_t : 1;
    } DEF_R8(modeswitch)

    struct [[nodiscard]] R8_Guesture : public Reg8<> {
        static constexpr RegAddr ADDRESS = 0x01;
        

        
        GestureId gesture_id;
    } DEF_R8(guesture)

    struct [[nodiscard]] R8_Status : public Reg8<> {
        static constexpr RegAddr ADDRESS = 0x02;
        
        uint8_t td_status : 4;
        uint8_t : 4;
    } DEF_R8(status)
};

}