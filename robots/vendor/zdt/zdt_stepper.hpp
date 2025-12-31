#pragma once


#include "zdt_stepper_transport.hpp"

namespace ymd::robots::zdtmotor{

class ZdtStepper final{
public:

    template<typename T = void>
    using IResult = IResult<T>;

    struct Config{
        NodeId node_id;
    };

    explicit ZdtStepper(const Config & cfg, Some<hal::Can *> && can) : 
        transport_(std::move(can)
    ){
        reconf(cfg);
    }

    explicit ZdtStepper(const Config & cfg, Some<hal::Uart *> && uart) : 
        transport_(std::move(uart)
    ){
        reconf(cfg);
    }


    void reconf(const Config & cfg){
        node_id_ = cfg.node_id;
    }

    struct PositionMsg{
        Angular<iq16> angle;
        iq16 speed;
    };

    struct SpeedMsg{
        iq16 speed;
    };

    IResult<> set_angle(const PositionMsg msg);
    IResult<> set_speed(const SpeedMsg msg);
    IResult<> brake();
    IResult<> set_subdivides(const uint16_t subdivides);
    IResult<> activate(const Enable en);
    IResult<> trig_cali();
    IResult<> query_homming_paraments();
    IResult<> trig_homming(const HommingMode mode);
private:
    using Phy = ZdtMotorPhy;
    Phy transport_;

    static constexpr auto DEFAULT_NODE_ID = NodeId::from_u8(0x01);
    NodeId node_id_ = DEFAULT_NODE_ID;

    bool is_multi_axis_sync_ = false;
    VerifyMethod verify_method_ = VerifyMethod::Default;



    template<typename T>
    [[nodiscard]] static constexpr FlatPacket req_to_flat_packet(
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

    template<typename T>
    IResult<> write_req_msg(const T & req_msg){

        transport_.write_flat_packet(
            req_to_flat_packet(
                node_id_, 
                verify_method_, 
                req_msg
            )
        );

        return Ok();
    }

};

}