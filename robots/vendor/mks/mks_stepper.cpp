#include "mks_stepper.hpp"

using namespace ymd;
using namespace ymd::robots::mksmotor;


IResult<> MksStepper::set_position(const MksStepper::PositionSetpoint msg){
    return write_payload(req_msgs::SetPositionMode3{
        .rpm = Rpm::from_tps(msg.speed),
        .acc_level = AcclerationLevel::from_tpss(msg.accerlation),
        .abs_pulse_cnt = PulseCnt::from_turns(msg.position)
    });
}

IResult<> MksStepper::set_speed(const MksStepper::SpeedSetpoint msg){
    return write_payload(req_msgs::SetSpeed{
        .rpm = iRpm::from_tps(msg.speed),
        .acc_level = AcclerationLevel::from_tpss(msg.accerlation)
    });
}

IResult<> MksStepper::brake(){
    TODO();
    return Ok();
}

IResult<> MksStepper::set_subdivides(const uint16_t subdivides){
    if(subdivides > 256) 
        return Err(Error::SubDivideOverflow);
    return write_payload(req_msgs::SetSubdivides{
        .subdivides = uint8_t(subdivides & 0xff)
    });
}

IResult<> MksStepper::activate(const Enable en){
    return write_payload(req_msgs::SetEnableStatus{
        .is_enabled = en == EN
    });
}

IResult<> MksStepper::trig_cali(){
    TODO();
    return Ok();
}

IResult<> MksStepper::query_homming_paraments(){
    TODO();
    return Ok();
}

IResult<> MksStepper::trig_homming(const HommingMode mode){
    TODO();
    return Ok();
}
