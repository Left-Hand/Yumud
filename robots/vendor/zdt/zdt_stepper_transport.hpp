#pragma once

#include "zdt_stepper_primitive.hpp"

namespace ymd::hal{
    class Can;
    class Uart;
};

namespace ymd::robots::zdtmotor{


class ZdtMotorPhy final{
public:

    ZdtMotorPhy(Some<hal::Can *> && can) : 
        may_uart_(ymd::None),
        may_can_(std::move(can)
    ){;}

    ZdtMotorPhy(Some<hal::Uart *> && uart) : 
        may_uart_(std::move(uart)),
        may_can_(ymd::None)
    {;}


    void write_flat_packet(
        const FlatPacket & flat_packet
    );
private:
    Option<hal::Uart &> may_uart_;
    Option<hal::Can &> may_can_;

    static void can_write_flat_packet(
        hal::Can & can, 
        const FlatPacket & flat_packet
    );

    static void uart_write_flat_packet(
        hal::Uart & uart, 
        const FlatPacket & flat_packet
    );
};



}