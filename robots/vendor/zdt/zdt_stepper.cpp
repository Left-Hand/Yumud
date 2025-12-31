#include "zdt_stepper.hpp"
#include "zdt_stepper_msg.hpp"

using namespace ymd;
using namespace ymd::robots::zdtmotor;

IResult<> ZdtStepper::set_angle(ZdtStepper::PositionMsg msg){
    return write_req_msg(req_msgs::SetPosition{
        .is_ccw = (msg.angle.is_negative()),
        .rpm = Rpm::from_speed(msg.speed),
        .acc_level = AcclerationLevel::from_u8(0),
        .pulse_cnt = PulseCnt::from_angle(msg.angle.abs().cast_inner<uq16>()).unwrap(),
        .is_absolute = true,
        .is_sync = is_multi_axis_sync_
    });
}

IResult<> ZdtStepper::set_speed(ZdtStepper::SpeedMsg msg){
    return write_req_msg(req_msgs::SetSpeed{
        .is_ccw = msg.speed < 0,
        .rpm = Rpm::from_speed(ABS(msg.speed)),
        .acc_level = AcclerationLevel::from(0),
        .is_absolute = true,
        .is_sync = is_multi_axis_sync_
    });
}

IResult<> ZdtStepper::brake(){
    return write_req_msg(req_msgs::Brake{
        .is_sync = is_multi_axis_sync_
    });
}

IResult<> ZdtStepper::set_subdivides(const uint16_t subdivides){
    if(subdivides > 256) 
        return Err(Error::SubDivideOverflow);
    return write_req_msg(req_msgs::SetSubDivides{
        .is_burned = false,
        .subdivides = uint8_t(subdivides & 0xff)
    });
}

IResult<> ZdtStepper::activate(const Enable en){
    return write_req_msg(req_msgs::Actvation{
        .en = en == EN,
        .is_sync = is_multi_axis_sync_
    });
}


IResult<> ZdtStepper::trig_cali(){
    return write_req_msg(req_msgs::TrigCali{});  
}

IResult<> ZdtStepper::query_homming_paraments(){
    return write_req_msg(req_msgs::QueryHommingParaments{});
}

IResult<> ZdtStepper::trig_homming(const HommingMode mode){
    return write_req_msg(req_msgs::TrigHomming{
        .homming_mode = mode,
        .is_sync = is_multi_axis_sync_
    });
}
