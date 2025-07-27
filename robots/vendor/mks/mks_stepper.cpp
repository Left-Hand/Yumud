#include"mks_stepper.hpp"

using namespace ymd;
using namespace ymd::robots::mksmotor;
using namespace ymd::robots::mksmotor::prelude;


IResult<> MksStepper::set_position(const MksStepper::PositionSetpoint pos){
    return write_payload(payloads::SetPositionMode3{
        .rpm = Rpm::from_speed(pos.speed),
        .acc_level = AcclerationLevel::from(pos.accerlation),
        .abs_pulse_cnt = PulseCnt::from_position(pos.position)
    });
}

IResult<> MksStepper::set_speed(const MksStepper::SpeedSetpoint spd){
    return write_payload(payloads::SetSpeed{
        .rpm = iRpm::from_speed(spd.speed),
        .acc_level = AcclerationLevel::from(spd.accerlation)
    });
}

IResult<> MksStepper::brake(){
    return Ok();
}

IResult<> MksStepper::set_subdivides(const uint16_t subdivides){
    if(subdivides > 256) 
        return Err(Error::SubDivideOverflow);
    return write_payload(payloads::SetSubdivides{
        .subdivides = uint8_t(subdivides & 0xff)
    });
}

IResult<> MksStepper::activate(const Enable en){
    return write_payload(payloads::SetEnableStatus{
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
