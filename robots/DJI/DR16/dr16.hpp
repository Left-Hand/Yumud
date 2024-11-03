#pragma once

#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uart.hpp"
#include "hal/adc/adc_channel.hpp"


namespace yumud::drivers{
class Mouse{
    
};

class Joystick{
    
};

class SBUS{
    
};

class DR16{
public:
    struct Frame{
        union{
            struct{
                uint16_t ch0:11;
                uint16_t ch1:11;
                uint16_t ch2:11;
                uint16_t ch3:11;
                uint16_t s1:2;
                uint16_t s2:2;
                uint16_t mouse_x:16;
                uint16_t mouse_y:16;
                uint16_t mouse_z:16;
                uint16_t mouse_l:8;
                uint16_t mouse_r:8;
                union{
                    struct{
                        uint16_t key_w:1;      
                        uint16_t key_a:1;      
                        uint16_t key_s:1;      
                        uint16_t key_d:1;      
                        uint16_t key_q:1;      
                        uint16_t key_e:1;      
                        uint16_t key_shift:1;      
                        uint16_t key_ctrl:1;      
                    };
                    uint16_t key:16;
                }__packed;
                uint16_t __resv__:16;
            };
            uint8_t data[18];
        }__packed;

        void reset(){
            memset(data, 0, sizeof(data));
        }
    };

protected:  
    using DBUS = Uart;
    DBUS & inst;
public:
    DR16(DBUS & _inst):inst(_inst){}
};

};