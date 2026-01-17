#pragma once

#include "mks_stepper_msgs.hpp"
#include "hal/bus/can/can.hpp"

namespace ymd::robots::mksmotor{


class MksMotorPhy final{
public:
    explicit MksMotorPhy(Some<hal::Can *> && can) : 
        uart_(ymd::None),
        can_(std::move(can)
    ){
        // reconf(cfg);
    }

    explicit MksMotorPhy(Some<hal::Uart *> && uart) : 
        uart_(std::move(uart)),
        can_(ymd::None)
    {
        // reconf(cfg);

    }


    void write_can_frame(const NodeId node_id, const std::span<const uint8_t> bytes) {
        const auto msg = hal::BxCanFrame(
            nodeid_to_canid(node_id),
            hal::BxCanPayload::from_bytes(bytes)
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

    static constexpr hal::CanStdId nodeid_to_canid(
        const NodeId node_id
    ){
        return hal::CanStdId::from_bits(node_id.to_u8());
    }
};
class MksMotor_CanTransport final{
public:
    explicit MksMotor_CanTransport(Some<hal::Can *> && can) : 
        can_(can.deref()){
        // reconf(cfg);
    }


    void write_msg(const NodeId node_id, const std::span<const uint8_t> bytes) {
        const auto msg = hal::BxCanFrame(
            nodeid_to_canid(node_id),
            hal::BxCanPayload::from_bytes(bytes)
        );

        can_.try_write(msg).examine();
    }


private:
    hal::Can & can_;

    static constexpr hal::CanStdId nodeid_to_canid(
        const NodeId node_id
    ){
        return hal::CanStdId::from_bits(node_id.to_u8());
    }
};



}