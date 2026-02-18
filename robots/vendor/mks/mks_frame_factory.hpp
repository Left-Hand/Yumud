#pragma once


#include "mks_stepper_msgs.hpp"

namespace ymd::robots::mksmotor{

class MksFrameFactory final{
public:

    NodeId node_id;

    struct PositionSetpoint{
        iq16 position;
        iq16 speed;
        iq16 accerlation = 0;
    };

    struct SpeedSetpoint{
        iq16 speed;
        iq16 accerlation = 0;
    };

    FlatPacket set_position(const PositionSetpoint pos);
    FlatPacket set_speed(const SpeedSetpoint spd);
    FlatPacket brake();
    FlatPacket set_subdivides(const uint16_t subdivides);
    FlatPacket activate(const Enable en);
    FlatPacket trig_cali();
    FlatPacket query_homming_paraments();
    FlatPacket trig_homming(const HommingMode mode);
private:
    template<typename Msg, typename T = std::decay_t<Msg>>
    constexpr FlatPacket ser_msg(
        Msg && msg
    ){
        return make_msg(node_id, std::forward<Msg>(msg));
    }


    template<typename Msg, typename T = std::decay_t<Msg>>
    static constexpr FlatPacket make_msg(
        const NodeId node_id,
        Msg && msg
    ){
        FlatPacket packet;

        const auto bytes = req_msgs::serialize(msg);
        const auto verify_code = get_verify_code(
            node_id,
            T::FUNC_CODE,
            bytes
        );

        packet.node_id = node_id;
        packet.buf.append_unchecked(std::bit_cast<uint8_t>(T::FUNC_CODE));
        packet.buf.append_unchecked(bytes);
        packet.buf.append_unchecked(verify_code);
        return packet;
    }
};

}