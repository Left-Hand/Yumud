#pragma once


#include "hal/gpio/gpio.hpp"
#include "hal/gpio/vport.hpp"
#include "drivers/CommonIO/Led/rgbLed.hpp"
#include "concept/pwm_channel.hpp"
#include "core/io/regs.hpp"
#include "types/vectors/vector2/vector2.hpp"

#include "hal/bus/spi/spidrv.hpp"


namespace ymd::drivers{
struct Ps2Joystick_Prelude{
    enum class JoyStickEvent:uint8_t{
        SELECT,
        L3,
        R3,
        START,
        UP,
        RIGHT,
        DOWN,
        LEFT,
        L2, 
        R2, 
        L1, 
        R1, 
        DELTA, 
        CIRC, 
        CROSS, 
        SQU,
        RX, 
        RY, 
        LX, 
        LY
    };


    enum class DevID:uint8_t{
        UNINIT = 0,
        NONE = 0x01,
        DIGIT = 0x41,
        NEGCON = 0x23,
        ANARED = 0x73,
        ANAGREEN = 0x53
    };

    enum class PressLevel:uint8_t{
        Pressed = 0,
        UnPress = 1
    };

    struct DataFrame{
        #pragma pack(push, 1)
        DevID dev_id = DevID::NONE;

        struct Modifiers{
            PressLevel select:1;
            PressLevel l3:1;
            PressLevel r3:1;
            PressLevel start:1;
            PressLevel up:1;
            PressLevel right:1;
            PressLevel down:1;
            PressLevel left:1;


            PressLevel l2:1;
            PressLevel r2:1;
            PressLevel l1:1;
            PressLevel r1:1;
            PressLevel delta:1;
            PressLevel circ:1;
            PressLevel cross:1;
            PressLevel squ:1;
        };

        static_assert(sizeof(Modifiers) == 2);

        Modifiers modifiers;

        uint8_t rx;
        uint8_t ry;
        uint8_t lx;
        uint8_t ly;

        #pragma pack(pop)

        std::span<uint8_t> to_bytes() {
            return std::span<uint8_t>(reinterpret_cast<uint8_t *>(this), sizeof(*this));
        }
    };

    DataFrame frame;

    static constexpr size_t FRAME_SIZE = sizeof(DataFrame);
    static_assert(FRAME_SIZE == 7);
};

class Ps2Joystick final:public Ps2Joystick_Prelude{
    
public:
    Ps2Joystick(hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    void init(){
        // Initialize the PS2 controller and set up the necessary pins
        // Configure the SPI interface and enable the controller
    }

    void update(){
        // Read and process data from the PS2 controller
        // Update the 'frame' struct with the new data

        DataFrame new_frame;
        spi_drv_.write_single<uint8_t>((uint8_t)0x01).unwrap_err();

        spi_drv_.transceive_single<uint8_t>(
            reinterpret_cast<uint8_t &>(frame.dev_id), 
            (uint8_t)0x42).unwrap_err();
        new_frame.dev_id = frame.dev_id;

        uint8_t permit;
        spi_drv_.transceive_single<uint8_t>(permit, uint8_t(0x00)).unwrap_err();

        spi_drv_.read_burst<uint8_t>(new_frame.to_bytes()).unwrap_err();

        if(permit == 0x5a){
            frame = new_frame;
        }
    }

    DevID id(){
        return frame.dev_id;
    }

    uint8_t query(const JoyStickEvent event){
        switch(event){
            case JoyStickEvent::SELECT:
                return PressLevel::Pressed == frame.modifiers.select;
            case JoyStickEvent::L3:
                return PressLevel::Pressed == frame.modifiers.l3;
            case JoyStickEvent::R3:
                return PressLevel::Pressed == frame.modifiers.r3;
            case JoyStickEvent::START:
                return PressLevel::Pressed == frame.modifiers.start;
            case JoyStickEvent::UP:
                return PressLevel::Pressed == frame.modifiers.up;
            case JoyStickEvent::RIGHT:
                return PressLevel::Pressed == frame.modifiers.right;
            case JoyStickEvent::DOWN:
                return PressLevel::Pressed == frame.modifiers.down;
            case JoyStickEvent::LEFT:
                return PressLevel::Pressed == frame.modifiers.left;


            case JoyStickEvent::L2:
                return PressLevel::Pressed == frame.modifiers.l2;
            case JoyStickEvent::R2:
                return PressLevel::Pressed == frame.modifiers.r2;
            case JoyStickEvent::L1:
                return PressLevel::Pressed == frame.modifiers.l1;
            case JoyStickEvent::R1:
                return PressLevel::Pressed == frame.modifiers.r1;
            case JoyStickEvent::DELTA:
                return PressLevel::Pressed == frame.modifiers.delta;
            case JoyStickEvent::CIRC:
                return PressLevel::Pressed == frame.modifiers.circ;
            case JoyStickEvent::CROSS:
                return PressLevel::Pressed == frame.modifiers.cross;
            case JoyStickEvent::SQU:
                return PressLevel::Pressed == frame.modifiers.squ;


            case JoyStickEvent::RX:
                return frame.rx;
            case JoyStickEvent::RY:
                return frame.ry;
            case JoyStickEvent::LX:
                return frame.lx;
            case JoyStickEvent::LY:
                return frame.ly;

            default:
                __builtin_unreachable();
        }
    }

    Vector2<real_t> get_left_joystick() const {
        return Vector2<real_t>{frame.lx-127, 127-frame.ly}/128;
    }

    Vector2<real_t> get_right_joystick() const {
        return Vector2<real_t>{frame.rx-127, 127-frame.ry}/128;
    }

    Vector2i get_left_direction() const{
        Vector2i dir;

        if(PressLevel::Pressed == frame.modifiers.left) 
            dir += Vector2i::LEFT;
        if(PressLevel::Pressed == frame.modifiers.right) 
            dir += Vector2i::RIGHT;
        if(PressLevel::Pressed == frame.modifiers.up) 
            dir += Vector2i::UP;
        if(PressLevel::Pressed == frame.modifiers.down) 
            dir += Vector2i::DOWN;

        // return dir;

        return dir;//why?
    }
private:
    hal::SpiDrv & spi_drv_;
};

}