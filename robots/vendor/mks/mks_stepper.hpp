#pragma once


#include "details/mks_stepper_utils.hpp"

namespace ymd::robots::mksmotor{

class MksStepper final{
public:
    using HommingMode = prelude::HommingMode;
    using Error = prelude::Error;
    using VerifyUtils = prelude::VerifyUtils;
    using Buf = prelude::Buf;

    template<typename T = void>
    using IResult = prelude::IResult<T>;

    struct Config{
        NodeId nodeid;
    };

    explicit MksStepper(const Config & cfg, Some<hal::Can *> && can) : 
        phy_(std::move(can)
    ){
        reconf(cfg);
    }

    explicit MksStepper(const Config & cfg, Some<hal::Uart *> && uart) : 
        phy_(std::move(uart)
    ){
        reconf(cfg);
    }


    void reconf(const Config & cfg){
        nodeid_ = cfg.nodeid;
    }

    struct PositionSetpoint{
        real_t position;
        real_t speed;
        real_t accerlation = 0;
    };

    struct SpeedSetpoint{
        real_t speed;
        real_t accerlation = 0;
    };

    IResult<> set_position(const PositionSetpoint pos);
    IResult<> set_speed(const SpeedSetpoint spd);
    IResult<> brake();
    IResult<> set_subdivides(const uint16_t subdivides);
    IResult<> activate(const Enable en);
    IResult<> trig_cali();
    IResult<> query_homming_paraments();
    IResult<> trig_homming(const HommingMode mode);
private:
    using Phy = MksMotorPhy;
    Phy phy_;

    static constexpr auto DEFAULT_NODE_ID = NodeId::from_u8(0x01);
    NodeId nodeid_ = DEFAULT_NODE_ID;

    template<typename Raw, typename T = std::decay_t<Raw>>
    static constexpr Buf map_payload_to_bytes(
        const NodeId nodeid,
        Raw && obj
    ){
        Buf buf;

        const auto bytes = payloads::serialize(obj);

        buf.append_unchecked(std::bit_cast<uint8_t>(T::FUNC_CODE));
        buf.append_unchecked(bytes);
        buf.append_unchecked(VerifyUtils::get_verify_code(
            nodeid,
            T::FUNC_CODE,
            bytes
        ));
        // buf.append_unchecked(0x34);
        return buf;
    }

    template<typename T>
    IResult<> write_payload(const T & obj){
        const auto buf = map_payload_to_bytes(nodeid_, obj);

        phy_.write_can_frame(nodeid_, buf.as_slice());

        return Ok();
    }

};

}