#include"mks_stepper.hpp"

using namespace ymd;
using namespace ymd::robots::mksmotor;
using namespace ymd::robots::mksmotor::prelude;


IResult<> MksStepper::set_position(const MksStepper::PositionSetpoint msg){
    return write_payload(msgs::SetPositionMode3{
        .rpm = Rpm::from_speed(msg.speed),
        .acc_level = AcclerationLevel::from(msg.accerlation),
        .abs_pulse_cnt = PulseCnt::from_position(msg.position)
    });
}

IResult<> MksStepper::set_speed(const MksStepper::SpeedSetpoint msg){
    return write_payload(msgs::SetSpeed{
        .rpm = iRpm::from_speed(msg.speed),
        .acc_level = AcclerationLevel::from(msg.accerlation)
    });
}

IResult<> MksStepper::brake(){
    return Ok();
}

IResult<> MksStepper::set_subdivides(const uint16_t subdivides){
    if(subdivides > 256) 
        return Err(Error::SubDivideOverflow);
    return write_payload(msgs::SetSubdivides{
        .subdivides = uint8_t(subdivides & 0xff)
    });
}

IResult<> MksStepper::activate(const Enable en){
    return write_payload(msgs::SetEnableStatus{
        .enable = en.to_bool()
    });
}

IResult<> MksStepper::trig_cali(){
    return Ok();
}

IResult<> MksStepper::query_homming_paraments(){
    return Ok();
}

IResult<> MksStepper::trig_homming(const HommingMode mode){
    return Ok();
}
