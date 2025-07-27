#pragma once


#include "details/zdt_stepper_utils.hpp"

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
        NodeId nodeid;
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
        nodeid_ = cfg.nodeid;
    }

    struct PositionSetpoint{
        real_t position;
        real_t speed;
    };

    struct SpeedSetpoint{
        real_t speed;
    };

    IResult<> set_position(const PositionSetpoint pos);
    IResult<> set_speed(const SpeedSetpoint spd);
    IResult<> brake();
    IResult<> set_subdivides(const uint16_t subdivides);
    IResult<> activate(const Enable en = EN);
    IResult<> trig_cali();
    IResult<> query_homming_paraments();
    IResult<> trig_homming(const HommingMode mode);
private:
    using Phy = ZdtMotorPhy;
    Phy phy_;

    static constexpr auto DEFAULT_NODE_ID = NodeId::from_u8(0x01);
    NodeId nodeid_ = DEFAULT_NODE_ID;

    bool is_sync_ = false;
    VerifyMethod verify_method_ = prelude::DEFAULT_VERIFY_METHOD;



    template<typename Raw, typename T = std::decay_t<Raw>>
    static constexpr Buf map_payload_to_bytes(
        const VerifyMethod verify_method,
        Raw && obj
    ){
        Buf buf;

        const auto bytes = payloads::serialize(obj);

        buf.append_unchecked(bytes);
        buf.append_unchecked(VerifyUtils::get_verify_code(
            verify_method,
            T::FUNC_CODE,
            bytes
        ));
        
        return buf;
    }

    template<typename T>
    IResult<> write_payload(const T & obj){
        const auto buf = map_payload_to_bytes(verify_method_, obj);

        phy_.write_bytes(
            nodeid_, 
            T::FUNC_CODE, 
            buf.iter()
        );

        return Ok();
    }

};

}