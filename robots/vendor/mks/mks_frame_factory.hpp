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

    constexpr FlatPacket set_position(const PositionSetpoint msg) const noexcept {
        return ser_msg(req_msgs::SetPositionMode3{
            .rpm = Rpm::from_tps(msg.speed),
            .acc_level = AcclerationLevel::from_tpss(msg.accerlation),
            .abs_pulse_cnt = PulseCnt::from_turns(msg.position)
        });
    }

    constexpr FlatPacket set_speed(const SpeedSetpoint msg) const noexcept {
        return ser_msg(req_msgs::SetSpeed{
            .rpm = iRpm::from_tps(msg.speed),
            .acc_level = AcclerationLevel::from_tpss(msg.accerlation)
        });
    }

    #if 0
    constexpr FlatPacket brake() const noexcept {
        TODO();

    }
    #endif

    constexpr FlatPacket set_subdivides(const uint16_t subdivides) const noexcept {
        return ser_msg(req_msgs::SetSubdivides{
            .subdivides = uint8_t(subdivides & 0xff)
        });
    }

    constexpr FlatPacket activate(const Enable en) const noexcept {
        return ser_msg(req_msgs::SetEnableStatus{
            .is_enabled = (en == EN)
        });
    }

    #if 0
    constexpr FlatPacket trig_cali() const noexcept {
        return ser_msg(req_msgs::SetEnableStatus{
            .is_enabled = (en == EN)
        });

    }

    constexpr FlatPacket query_homming_paraments() const noexcept {
        return ser_msg(req_msgs::SetEnableStatus{
            .is_enabled = (en == EN)
        });

    }

    constexpr FlatPacket trig_homming(const HommingMode mode) const noexcept {
        return ser_msg(req_msgs::SetEnableStatus{
            .is_enabled = (en == EN)
        });
    }
    #endif

private:
    template<typename Msg, typename T = std::decay_t<Msg>>
    constexpr FlatPacket ser_msg(Msg && msg) const noexcept {
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