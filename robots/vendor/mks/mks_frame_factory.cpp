#include "mks_frame_factory.hpp"

using namespace ymd;
using namespace ymd::robots::mksmotor;


FlatPacket MksFrameFactory::set_position(const MksFrameFactory::PositionSetpoint msg){
    return ser_msg(req_msgs::SetPositionMode3{
        .rpm = Rpm::from_tps(msg.speed),
        .acc_level = AcclerationLevel::from_tpss(msg.accerlation),
        .abs_pulse_cnt = PulseCnt::from_turns(msg.position)
    });
}

FlatPacket MksFrameFactory::set_speed(const MksFrameFactory::SpeedSetpoint msg){
    return ser_msg(req_msgs::SetSpeed{
        .rpm = iRpm::from_tps(msg.speed),
        .acc_level = AcclerationLevel::from_tpss(msg.accerlation)
    });
}

FlatPacket MksFrameFactory::brake(){
    TODO();

}

FlatPacket MksFrameFactory::set_subdivides(const uint16_t subdivides){
    return ser_msg(req_msgs::SetSubdivides{
        .subdivides = uint8_t(subdivides & 0xff)
    });
}

FlatPacket MksFrameFactory::activate(const Enable en){
    return ser_msg(req_msgs::SetEnableStatus{
        .is_enabled = en == EN
    });
}

FlatPacket MksFrameFactory::trig_cali(){
    TODO();

}

FlatPacket MksFrameFactory::query_homming_paraments(){
    TODO();

}

FlatPacket MksFrameFactory::trig_homming(const HommingMode mode){
    TODO();

}
