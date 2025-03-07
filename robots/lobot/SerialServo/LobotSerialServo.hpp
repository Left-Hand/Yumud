#pragma once

#include "drivers/device_defs.h"

namespace ymd::drivers{

class LobotSerialServo{
protected:
    hal::Uart & uart_;
    uint8_t id_;

    template<typename T>
    void writeFrame(const T & data){
        uart_.writeN(reinterpret_cast<const char *>(&data), sizeof(data));
    };

public:
    LobotSerialServo(hal::Uart & uart, const uint8_t id):
            uart_(uart), id_(id){}

    void setID(const uint8_t newID);

    void move(const int16_t position,const uint16_t time);

    void unload();

    void load();
};

}