#pragma once

#include "zdt_stepper_prelude.hpp"

namespace ymd::robots::zdtmotor{


class ZdtMotorPhy final{
public:
    using NodeId = prelude::NodeId;
    using FuncCode = prelude::FuncCode;
    

    ZdtMotorPhy(Some<hal::Can *> && can) : 
        uart_(ymd::None),
        can_(std::move(can)
    ){
        // reconf(cfg);
    }

    ZdtMotorPhy(Some<hal::Uart *> && uart) : 
        uart_(std::move(uart)),
        can_(ymd::None)
    {
        // reconf(cfg);

    }


    void write_bytes(
        const NodeId id, 
        const FuncCode func_code,
        const std::span<const uint8_t> bytes
    );
private:
    Option<hal::Uart &> uart_;
    Option<hal::Can &> can_;

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

struct ZdtMotorCanSink{

};

struct ZdtMotorCanSource{

};


}