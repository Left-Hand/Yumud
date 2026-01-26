#pragma once

#include "core/io/regs.hpp"

#include "hal/bus/uart/uart.hpp"

namespace ymd::robots::lobot{

class LobotSerialServo{
protected:
    hal::UartBase & uart_;
    uint8_t id_;

    template<typename T>
    void write_frame(const T & data){
        uart_.try_write_bytes(std::span<const uint8_t>(
            reinterpret_cast<const uint8_t *>(&data),
            sizeof(data)
        ));
    };

public:
    LobotSerialServo(hal::UartBase & uart, const uint8_t id):
            uart_(uart), id_(id){}

    void set_id(const uint8_t newID);

    void move(const int16_t position,const uint16_t time);

    void unload();

    void load();
};

}
