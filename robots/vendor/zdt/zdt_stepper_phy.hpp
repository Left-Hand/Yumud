#pragma once

#include "zdt_stepper_prelude.hpp"

namespace ymd::robots::zdtmotor{


class ZdtMotorPhy final{
public:
    using FuncCode = prelude::FuncCode;

    ZdtMotorPhy(Some<hal::Can *> && can) : 
        may_uart_(ymd::None),
        may_can_(std::move(can)
    ){;}

    ZdtMotorPhy(Some<hal::Uart *> && uart) : 
        may_uart_(std::move(uart)),
        may_can_(ymd::None)
    {;}


    void write_bytes(
        const NodeId id, 
        const FuncCode func_code,
        const std::span<const uint8_t> bytes
    );
private:
    Option<hal::Uart &> may_uart_;
    Option<hal::Can &> may_can_;

    static void can_write_bytes(
        hal::Can & can, 
        const NodeId id, 
        const FuncCode func_code,
        const std::span<const uint8_t> bytes
    );

    static void uart_write_bytes(
        hal::Uart & uart, 
        const NodeId id, 
        const FuncCode func_code,
        const std::span<const uint8_t> bytes
    );
};



}