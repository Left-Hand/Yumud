#pragma once

#include "drivers/device_defs.h"
#include "hal/gpio/gpio.hpp"


namespace ymd::drivers{
class JQ8900{
    private:
        Gpio & ser;

        enum class CMD:uint8_t{
            CLR = 0x0A,
            SET_DISC = 0x0B,
            SET_VOL = 0x0C,
            SET_EQ = 0x0D,
        };

        void send(const uint8_t data);
        void sendLine(const uint8_t value, const CMD cmd);
    public:
        JQ8900(Gpio & _ser):ser(_ser){};

        void init(){;}


        void setVol(const uint8_t value){sendLine(value, CMD::SET_VOL);}
        void setDisc(const uint8_t value){sendLine(value, CMD::SET_DISC);}

};

}