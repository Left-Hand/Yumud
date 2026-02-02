#include "zdt_frame_factory.hpp"
#include "zdt_stepper_msg.hpp"

using namespace ymd;
using namespace ymd::robots::zdtmotor;



FlatPacket ZdtFrameFactory::set_angle(const Angular<iq16> angle, iq16 speed){
    return ser_req(req_msgs::SetPosition{
        .is_ccw = (angle.is_negative()),
        .rpm = Rpm::from_speed(speed),
        .acc_level = AcclerationLevel::from_u8(0),
        .pulse_cnt = PulseCnt::from_angle(angle.abs().cast_inner<uq16>()).unwrap(),
        .is_absolute = true,
        .is_sync = is_multi_axis_sync_
    });
}

FlatPacket ZdtFrameFactory::set_speed(iq16 speed){
    return ser_req(req_msgs::SetSpeed{
        .is_ccw = speed < 0,
        .rpm = Rpm::from_speed(math::abs(speed)),
        .acc_level = AcclerationLevel::from(0),
        .is_absolute = true,
        .is_sync = is_multi_axis_sync_
    });
}

FlatPacket ZdtFrameFactory::brake(){
    return ser_req(req_msgs::Brake{
        .is_sync = is_multi_axis_sync_
    });
}

FlatPacket ZdtFrameFactory::set_subdivides(const uint16_t subdivides){
    return ser_req(req_msgs::SetSubDivides{
        .is_burned = false,
        .subdivides = uint8_t(subdivides & 0xff)
    });
}

FlatPacket ZdtFrameFactory::activate(const Enable en){
    return ser_req(req_msgs::Actvation{
        .en = en == EN,
        .is_sync = is_multi_axis_sync_
    });
}


FlatPacket ZdtFrameFactory::trig_cali(){
    return ser_req(req_msgs::TrigCali{});  
}

FlatPacket ZdtFrameFactory::query_homming_paraments(){
    return ser_req(req_msgs::QueryHommingParaments{});
}

FlatPacket ZdtFrameFactory::trig_homming(const HommingMode mode){
    return ser_req(req_msgs::TrigHomming{
        .homming_mode = mode,
        .is_sync = is_multi_axis_sync_
    });
}

