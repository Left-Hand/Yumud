#pragma once

#include "mks_stepper_prelude.hpp"

namespace ymd::robots::mksmotor{


class MksMotorPhy final{
public:
    using FuncCode = prelude::FuncCode;
    

    MksMotorPhy(Some<hal::Can *> && can) : 
        uart_(ymd::None),
        can_(std::move(can)
    ){
        // reconf(cfg);
    }

    MksMotorPhy(Some<hal::Uart *> && uart) : 
        uart_(std::move(uart)),
        can_(ymd::None)
    {
        // reconf(cfg);

    }


    void write_can_msg(const NodeId nodeid, const std::span<const uint8_t> bytes) {
        const auto msg = hal::CanClassicMsg::from_bytes(
            map_nodeid_to_canid(nodeid),
            bytes
        );

        if(can_.is_some()){
            // can_.unwrap().write(msg).examine();

            DEBUG_PRINTLN(msg);
        }else{
            PANIC("uart_ is None");
        }
    }


private:
    Option<hal::Uart &> uart_;
    Option<hal::Can &> can_;

    static constexpr hal::CanStdId map_nodeid_to_canid(
        const NodeId nodeid
    ){
        return hal::CanStdId::from_bits(nodeid.to_u8());
    }
};



}