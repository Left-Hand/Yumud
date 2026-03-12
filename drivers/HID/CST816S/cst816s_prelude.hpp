#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/utils/nth.hpp"
#include "core/string/owned/char_array.hpp"

#include "algebra/vectors/vec2.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

//参考资料:
// BSD 3-Clause License
// https://github.com/tstellanova/cst816s/blob/main/src/lib.rs#L159


namespace ymd::drivers{


struct [[nodiscard]] CST816S_Prelude{
protected:
    using RegAddr = uint16_t;
    static constexpr hal::I2cSlaveAddr<7> DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x15);

    static constexpr size_t MAX_NUM_TOUCHPOINTS  = 5;


    static constexpr size_t GESTURE_HEADER_LEN = 3;
    /// Number of bytes for a single touch event
    static constexpr size_t RAW_TOUCH_EVENT_LEN = 6;

    /// In essence, max number of fingers
    static constexpr size_t MAX_TOUCH_CHANNELS = 10;



public:


    struct [[nodiscard]] GestureId{
        enum class [[nodiscard]] Kind:uint8_t{
            SlideDown = 0x01,
            SlideUp = 0x02,
            SlideLeft = 0x03,
            SlideRight = 0x04,
            SingleClick = 0x05,
            DoubleClick = 0x0B,
            LongPress = 0x0C,
        };

        DEF_FRIEND_DERIVE_DEBUG(Kind)

        using enum Kind;

        constexpr GestureId(Kind kind):kind_(std::bit_cast<uint8_t>(kind)) {;}
        constexpr GestureId(_None_t):kind_(0) {;}

        static constexpr Option<GestureId> from_u8(const uint8_t b){
            switch(b){
                case static_cast<uint8_t>(Kind::SlideDown): 
                case static_cast<uint8_t>(Kind::SlideUp):   
                case static_cast<uint8_t>(Kind::SlideLeft): 
                case static_cast<uint8_t>(Kind::SlideRight):    
                case static_cast<uint8_t>(Kind::SingleClick):   
                case static_cast<uint8_t>(Kind::DoubleClick):   
                case static_cast<uint8_t>(Kind::LongPress): {
                    return Some(GestureId(static_cast<Kind>(b)));
                } 
            }
            return None;
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


protected:


    struct [[nodiscard]] TouchEvent{

        uint16_t x;
        uint16_t y;
        GestureId gesture;
        uint8_t action;
        uint8_t finger_id;
        uint8_t pressure;
        uint8_t area;



        static constexpr Option<TouchEvent> try_from_bytes(std::span<const uint8_t, RAW_TOUCH_EVENT_LEN> bytes) {
            const uint8_t touch_x_h_and_action = bytes[TOUCH_X_H_AND_ACTION_OFF];
            const uint8_t touch_y_h_and_finger = bytes[TOUCH_Y_H_AND_FINGER_OFF];

            const uint32_t x = static_cast<uint32_t>(bytes[TOUCH_X_L_OFF]) |
                (static_cast<uint32_t>(touch_x_h_and_action & 0x0F) << 8);

            const uint32_t y = static_cast<uint32_t>(bytes[TOUCH_Y_L_OFF]) |
                (static_cast<uint32_t>(touch_y_h_and_finger & 0x0F) << 8);
            const auto touch = TouchEvent{
                .x = static_cast<uint16_t>(x),
                .y = static_cast<uint16_t>(y),
                .gesture = GestureId(None), // Default to None
                .action = static_cast<uint8_t>(touch_x_h_and_action >> 6),
                .finger_id = static_cast<uint8_t>(touch_y_h_and_finger >> 4),
                .pressure = bytes[TOUCH_PRESURE_OFF],
                .area = static_cast<uint8_t>(bytes[TOUCH_AREA_OFF] >> 4)
            };
            return Some(touch);
        }
    };


    /// The first register on the device
    static constexpr uint8_t REG_FIRST = 0x00;
    /// Header bytes (first three of every register block read)
    // const RESERVED_0_OFF: usize = 0;
    static constexpr size_t GESTURE_ID_OFF = 1;
    static constexpr size_t NUM_POINTS_OFF = 2;

    /// These offsets are relative to the body start (after NUM_POINTS_OFF)
    /// offset of touch X position high bits and Action bits
    static constexpr size_t TOUCH_X_H_AND_ACTION_OFF = 0;
    /// offset of touch X position low bits
    static constexpr size_t TOUCH_X_L_OFF = 1;
    /// offset of touch Y position high bits and Finger bits
    static constexpr size_t TOUCH_Y_H_AND_FINGER_OFF = 2;
    /// offset of touch Y position low bits
    static constexpr size_t TOUCH_Y_L_OFF = 3;
    static constexpr size_t TOUCH_PRESURE_OFF = 4;
    static constexpr size_t TOUCH_AREA_OFF = 5;
};


struct [[nodiscard]] CST816S_Regs:public CST816S_Prelude{
    struct [[nodiscard]] R32_ProductId{
        static constexpr RegAddr NUM_ADDRESS = 0x8140;
        std::array<uint8_t, 4> id;

        constexpr bool is_valid() const {
            return id[0] == '9' && id[1] == '1' && id[2] == '1' && id[3] == '\0';
        }
    };

    struct [[nodiscard]] R16_Firmware{
        static constexpr RegAddr NUM_ADDRESS = 0x8144;

        uint8_t low;
        uint8_t high;
    };

    struct [[nodiscard]] R32_Resolution{
        static constexpr RegAddr NUM_ADDRESS = 0x8146;

        uint16_t x;
        uint16_t y;
    };

    struct [[nodiscard]] R8_VendorId{
        static constexpr RegAddr NUM_ADDRESS = 0x8148;

        uint8_t id;
    };

    struct [[nodiscard]] R8_Status{
        static constexpr RegAddr ADDRES = 0x814e;

        uint8_t points_count:4;
        uint8_t :2;
        uint8_t large_detect:1;
        uint8_t buffer_status:1;
    };
};
}