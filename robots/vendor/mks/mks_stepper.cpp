#include"mks_stepper.hpp"

using namespace ymd;
using namespace ymd::robots::mksmotor;
using namespace ymd::robots::mksmotor::prelude;


IResult<> MksStepper::set_position(const MksStepper::PositionSetpoint pos){
    return write_payload(payloads::SetPositionMode3{
        .rpm = Rpm::from_speed(pos.speed),
        .acc_level = AcclerationLevel::from(0),
        .abs_pulse_cnt = PulseCnt::from_position(pos.position)
    });
}

IResult<> MksStepper::set_speed(const MksStepper::SpeedSetpoint spd){
    return Ok();
}

IResult<> MksStepper::brake(){
    return Ok();
}

IResult<> MksStepper::set_subdivides(const uint16_t subdivides){
    return Ok();
}

IResult<> MksStepper::activate(const Enable en){
    return Ok();
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
