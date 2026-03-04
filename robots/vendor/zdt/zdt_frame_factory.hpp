#pragma once


#include "zdt_stepper_utils.hpp"
#include "zdt_stepper_msg.hpp"

namespace ymd::robots::zdtmotor{


class ZdtFrameFactory final{
public:
    static constexpr auto DEFAULT_NODE_ID = NodeId::from_u8(0x01);
    NodeId node_id;
    VerifyMethod verify_method = VerifyMethod::Default;
    bool is_multi_axis_sync = false;


    constexpr FlatPacket set_angle(const Angular<iq16> angle, iq16 speed) const {
        return ser_req(req_msgs::SetPosition{
            .is_ccw = (angle.is_negative()),
            .rpm = Rpm::from_tps(speed),
            .acc_level = AcclerationLevel::from_u8(0),
            .pulse_cnt = PulseCnt::from_angle(angle.abs().cast_inner<uq16>()).unwrap(),
            .is_absolute = true,
            .is_sync = is_multi_axis_sync
        });
    }

    constexpr FlatPacket set_speed(iq16 speed) const {
        return ser_req(req_msgs::SetSpeed{
            .is_ccw = speed < 0,
            .rpm = Rpm::from_tps(math::abs(speed)),
            .acc_level = AcclerationLevel::from_tpss(0),
            .is_absolute = true,
            .is_sync = is_multi_axis_sync
        });
    }

    constexpr FlatPacket brake() const {
        return ser_req(req_msgs::Brake{
            .is_sync = is_multi_axis_sync
        });
    }

    constexpr FlatPacket set_subdivides(const uint16_t subdivides) const {
        return ser_req(req_msgs::SetSubDivides{
            .is_burned = false,
            .subdivides = uint8_t(subdivides & 0xff)
        });
    }

    constexpr FlatPacket activate(const Enable en) const {
        return ser_req(req_msgs::Actvation{
            .en = en == EN,
            .is_sync = is_multi_axis_sync
        });
    }


    constexpr FlatPacket trig_cali() const {
        return ser_req(req_msgs::TrigCali{});  
    }

    constexpr FlatPacket query_homming_paraments() const {
        return ser_req(req_msgs::QueryHommingParaments{});
    }

    constexpr FlatPacket trig_homming(const HommingMode mode) const {
        return ser_req(req_msgs::TrigHomming{
            .homming_mode = mode,
            .is_sync = is_multi_axis_sync
        });
    }


private:
template<typename T>
    [[nodiscard]] constexpr FlatPacket ser_req(const T & req) const {
        return make_req(node_id, verify_method, req);
    }


    template<typename T>
    [[nodiscard]] static constexpr FlatPacket make_req(
        const NodeId node_id,   
        const VerifyMethod verify_method,
        const T & req_msg
    ) {
        constexpr FuncCode FUNC_CODE = std::decay_t<T>::FUNC_CODE;
        constexpr size_t PAYLOAD_LENGTH = std::decay_t<T>::PAYLOAD_LENGTH;

        FlatPacket flat_packet;
        static_assert(PAYLOAD_LENGTH + 1 <= std::size(flat_packet.context));

        flat_packet.node_id = node_id;
        flat_packet.func_code = FUNC_CODE;

        const auto payload_bytes = std::span<uint8_t, PAYLOAD_LENGTH>(
            &flat_packet.context[0],
            PAYLOAD_LENGTH
        );

        if constexpr (PAYLOAD_LENGTH > 0){
            req_msg.fill_bytes(payload_bytes);
        }
        flat_packet.context[PAYLOAD_LENGTH] = VerifyUtils::get_verify_code(
            verify_method,
            FUNC_CODE,
            payload_bytes
        );
        flat_packet.payload_len = PAYLOAD_LENGTH;

        return flat_packet;
    }

};

}