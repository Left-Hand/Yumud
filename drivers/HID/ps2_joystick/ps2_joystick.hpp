#pragma once


#include "hal/gpio/gpio.hpp"
#include "hal/gpio/port_virtual.hpp"
#include "drivers/CommonIO/Led/rgbLed.hpp"
#include "hal/timer/pwm/pwm_channel.hpp"
#include "drivers/device_defs.h"
#include "types/vector2/vector2_t.hpp"

class Ps2Joystick{
    // struct Ps2Interface{
    //     Ps2Interface(GpioConcept & _sck_pin, GpioConcept & _sdo_pin):sck_pin(_sck_pin), sdo_pin(_sdo_pin)
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
    
    SpiDrv & bus_drv;
public:
    Ps2Joystick(SpiDrv & spi_drv):bus_drv(spi_drv){;}
    void init(){
        // Initialize the PS2 controller and set up the necessary pins
        // Configure the SPI interface and enable the controller
    }

    uint8_t permit;
    void update(){
        // Read and process data from the PS2 controller
        // Update the 'frame' struct with the new data

        DataFrame new_frame;
        bus_drv.write((uint8_t)0x01, false);

        bus_drv.transfer(*(uint8_t *)&frame.dev_id, (uint8_t)0x42, false);
        new_frame.dev_id = frame.dev_id;

        bus_drv.transfer(permit, (uint8_t)0x00, false);


        for(uint8_t i = 0; i < 6; i++){
            bus_drv.transfer(new_frame.data[i], (uint8_t)0x00, i == 5);
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
                return frame.select;
            case JoyStickEvent::L3:
                return frame.l3;
            case JoyStickEvent::R3:
                return frame.r3;
            case JoyStickEvent::START:
                return frame.start;
            case JoyStickEvent::UP:
                return frame.up;
            case JoyStickEvent::RIGHT:
                return frame.right;
            case JoyStickEvent::DOWN:
                return frame.down;
            case JoyStickEvent::LEFT:
                return frame.left;


            case JoyStickEvent::L2:
                return frame.l2;
            case JoyStickEvent::R2:
                return frame.r2;
            case JoyStickEvent::L1:
                return frame.l1;
            case JoyStickEvent::R1:
                return frame.r1;
            case JoyStickEvent::DELTA:
                return frame.delta;
            case JoyStickEvent::CIRC:
                return frame.circ;
            case JoyStickEvent::CROSS:
                return frame.cross;
            case JoyStickEvent::SQU:
                return frame.squ;


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

    Vector2i getLeftJoystick() const {
        return Vector2i{frame.lx, frame.ly};
    }

    Vector2i getRightJoystick() const {
        return Vector2i{frame.rx, frame.ry};
    }

    Vector2i getLeftDirection() const{
        Vector2i dir;

        if(frame.left) dir += Vector2i::LEFT;
        if(frame.right) dir += Vector2i::RIGHT;
        if(frame.up) dir += Vector2i::UP;
        if(frame.down) dir += Vector2i::DOWN;

        return dir;
    }
};