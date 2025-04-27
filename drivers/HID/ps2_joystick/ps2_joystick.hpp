#pragma once


#include "hal/gpio/gpio.hpp"
#include "hal/gpio/vport.hpp"
#include "drivers/CommonIO/Led/rgbLed.hpp"
#include "concept/pwm_channel.hpp"
#include "core/io/regs.hpp"
#include "types/vector2/vector2.hpp"

#include "hal/bus/spi/spidrv.hpp"


namespace ymd::drivers{
class Ps2Joystick{
    // struct Ps2Interface{
    //     Ps2Interface(hal::GpioIntf & _sck_pin, hal::GpioIntf & _sdo_pin):sck_pin(_sck_pin), sdo_pin(_sdo_pin)
    // };
public:
    enum class JoyStickEvent:uint8_t{
        // GREEN,
        // RED,
        SELECT,L3,R3,START,UP,RIGHT,DOWN,LEFT,
        L2, R2, L1, R1, DELTA, CIRC, CROSS, SQU,
        RX, RY, LX, LY
    };


    enum class DevID:uint8_t{
        UNINIT = 0,
        NONE = 0x01,
        DIGIT = 0x41,
        NEGCON = 0x23,
        ANARED = 0x73,
        ANAGREEN = 0x53
    };

protected:

    struct DataFrame{
        DevID dev_id = DevID::NONE;
        union{
            struct{
                uint8_t select:1;
                uint8_t l3:1;
                uint8_t r3:1;
                uint8_t start:1;
                uint8_t up:1;
                uint8_t right:1;
                uint8_t down:1;
                uint8_t left:1;


                uint8_t l2:1;
                uint8_t r2:1;
                uint8_t l1:1;
                uint8_t r1:1;
                uint8_t delta:1;
                uint8_t circ:1;
                uint8_t cross:1;
                uint8_t squ:1;

                uint8_t rx;
                uint8_t ry;
                uint8_t lx;
                uint8_t ly;
            };

            uint8_t data[6] = {0};
        };
    };

    DataFrame frame;
    
    hal::SpiDrv & spi_drv_;
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
        spi_drv_.write_single((uint8_t)0x01).unwrap_err();

        spi_drv_.transfer_single(*(uint8_t *)&frame.dev_id, (uint8_t)0x42).unwrap_err();
        new_frame.dev_id = frame.dev_id;

        uint8_t permit;
        spi_drv_.transfer_single(permit, (uint8_t)0x00).unwrap_err();


        for(uint8_t i = 0; i < 6; i++){
            spi_drv_.transfer_single(new_frame.data[i], (uint8_t)0x00, Continuous::from(i == 5)).unwrap_err();
        }

        if(permit == 0x5a){
            frame = new_frame;
        }
    }

    DevID id(){
        return frame.dev_id;
    }

    uint8_t valueof(const JoyStickEvent event){
        switch(event){
            case JoyStickEvent::SELECT:
                return not frame.select;
            case JoyStickEvent::L3:
                return not frame.l3;
            case JoyStickEvent::R3:
                return not frame.r3;
            case JoyStickEvent::START:
                return not frame.start;
            case JoyStickEvent::UP:
                return not frame.up;
            case JoyStickEvent::RIGHT:
                return not frame.right;
            case JoyStickEvent::DOWN:
                return not frame.down;
            case JoyStickEvent::LEFT:
                return not frame.left;


            case JoyStickEvent::L2:
                return not frame.l2;
            case JoyStickEvent::R2:
                return not frame.r2;
            case JoyStickEvent::L1:
                return not frame.l1;
            case JoyStickEvent::R1:
                return not frame.r1;
            case JoyStickEvent::DELTA:
                return not frame.delta;
            case JoyStickEvent::CIRC:
                return not frame.circ;
            case JoyStickEvent::CROSS:
                return not frame.cross;
            case JoyStickEvent::SQU:
                return not frame.squ;


            case JoyStickEvent::RX:
                return frame.rx;
            case JoyStickEvent::RY:
                return frame.ry;
            case JoyStickEvent::LX:
                return frame.lx;
            case JoyStickEvent::LY:
                return frame.ly;
        }
        return 0;
    }

    Vector2_t<real_t> getLeftJoystick() const {
        return Vector2_t<real_t>{frame.lx-127, 127-frame.ly}/128;
    }

    Vector2_t<real_t> getRightJoystick() const {
        return Vector2_t<real_t>{frame.rx-127, 127-frame.ry}/128;
    }

    Vector2i getLeftDirection() const{
        Vector2i dir;

        if(not frame.left) dir += Vector2i::LEFT;
        if(not frame.right) dir += Vector2i::RIGHT;
        if(not frame.up) dir += Vector2i::UP;
        if(not frame.down) dir += Vector2i::DOWN;

        // return dir;

        return dir;//why?
    }
};

}