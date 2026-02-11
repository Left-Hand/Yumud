#pragma once


#include "zdt_stepper_utils.hpp"

namespace ymd::robots::zdtmotor{


class ZdtFrameFactory final{
public:
    static constexpr auto DEFAULT_NODE_ID = NodeId::from_u8(0x01);
    NodeId node_id;
    VerifyMethod verify_method = VerifyMethod::Default;
    bool is_multi_axis_sync = false;


    FlatPacket brake();
    FlatPacket set_subdivides(const uint16_t subdivides);
    FlatPacket activate(const Enable en);
    FlatPacket trig_cali();
    FlatPacket query_homming_paraments();
    FlatPacket trig_homming(const HommingMode mode);
    FlatPacket set_angle(const Angular<iq16> position, iq16 speed);
    FlatPacket set_speed(iq16 speed);

private:
template<typename T>
    [[nodiscard]] constexpr FlatPacket ser_req(
        const T & req){
        return make_req(node_id, verify_method, req);
    }


    template<typename T>
    [[nodiscard]] static constexpr FlatPacket make_req(
        const NodeId node_id,   
        const VerifyMethod verify_method,
        const T & req_msg
    ){
        constexpr FuncCode FUNC_CODE = std::decay_t<T>::FUNC_CODE;
        constexpr size_t PAYLOAD_LENGTH = std::decay_t<T>::PAYLOAD_LENGTH;

        FlatPacket flat_packet;
        static_assert(PAYLOAD_LENGTH + 1 <= std::size(flat_packet.context));

        flat_packet.node_id = node_id;
        flat_packet.func_code = std::decay_t<T>::FUNC_CODE;

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