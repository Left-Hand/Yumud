#pragma once


#include "zdt_stepper_phy.hpp"

namespace ymd::robots::zdtmotor{

class ZdtStepper final{
public:
    using HommingMode = prelude::HommingMode;
    using VerifyMethod = prelude::VerifyMethod;
    using Buf = prelude::Buf;
    using Error = prelude::Error;
    using VerifyUtils = prelude::VerifyUtils;

    template<typename T = void>
    using IResult = prelude::IResult<T>;

    struct Config{
        NodeId node_id;
    };

    explicit ZdtStepper(const Config & cfg, Some<hal::Can *> && can) : 
        phy_(std::move(can)
    ){
        reconf(cfg);
    }

    explicit ZdtStepper(const Config & cfg, Some<hal::Uart *> && uart) : 
        phy_(std::move(uart)
    ){
        reconf(cfg);
    }


    void reconf(const Config & cfg){
        node_id_ = cfg.node_id;
    }

    struct PositionMsg{
        Angle<iq16> position;
        iq16 speed;
    };

    struct SpeedMsg{
        iq16 speed;
    };

    IResult<> set_position(const PositionMsg msg);
    IResult<> set_speed(const SpeedMsg msg);
    IResult<> brake();
    IResult<> set_subdivides(const uint16_t subdivides);
    IResult<> activate(const Enable en);
    IResult<> trig_cali();
    IResult<> query_homming_paraments();
    IResult<> trig_homming(const HommingMode mode);
private:
    using Phy = ZdtMotorPhy;
    Phy phy_;

    static constexpr auto DEFAULT_NODE_ID = NodeId::from_u8(0x01);
    NodeId node_id_ = DEFAULT_NODE_ID;

    bool is_multi_axis_sync_ = false;
    VerifyMethod verify_method_ = VerifyMethod::Default;



    template<typename T>
    [[nodiscard]] static constexpr Buf map_payload_to_bytes(
        const VerifyMethod verify_method,
        T && payload
    ){
        Buf buf;

        const auto bytes = payloads::serialize(payload);

        buf.append_unchecked(bytes);
        buf.append_unchecked(VerifyUtils::get_verify_code(
            verify_method,
            std::decay_t<T>::FUNC_CODE,
            bytes
        ));
        
        return buf;
    }

    template<typename T>
    IResult<> write_payload(const T & payload){
        const auto buf = map_payload_to_bytes(verify_method_, payload);

        phy_.write_bytes(
            node_id_, 
            T::FUNC_CODE, 
            buf.as_slice()
        );

        return Ok();
    }

};

}