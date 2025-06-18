#pragma once

#include "details/zdt_stepper_utils.hpp"

namespace ymd::drivers{

class ZdtStepper final:
    public ZdtMotor_Collections{
public:
    struct Config{
        NodeId nodeid;
    };


    ZdtStepper(const Config & cfg, Some<hal::Can *> && can) : 
        phy_(std::move(can)
    ){
        reconf(cfg);
    }

    ZdtStepper(const Config & cfg, Some<hal::Uart *> && uart) : 
        phy_(std::move(uart)
    ){
        reconf(cfg);
    }


    void reconf(const Config & cfg){
        nodeid_ = cfg.nodeid;
    }

    void set_target_position(const real_t pos);
    void set_target_speed(const real_t spd);
    void brake();
    void set_subdivides(const uint16_t subdivides);
    void enable(const Enable en = EN);
    void trigger_cali();
    void query_homming_paraments();
    void trig_homming(const HommingMode mode);
private:
    using Phy = ZdtMotorPhy;
    Phy phy_;

    static constexpr auto DEFAULT_NODE_ID = NodeId::from_u8(0x01);
    NodeId nodeid_ = DEFAULT_NODE_ID;

    bool is_sync_ = false;
    VerifyMethod verify_method_ = DEFAULT_VERIFY_METHOD;



    template<typename Raw, typename T = std::decay_t<Raw>>
    static constexpr Buf map_payload_to_bytes(
        const VerifyMethod verify_method,
        Raw && obj
    ){
        Buf buf;

        const auto bytes = Payloads::serialize(obj);

        buf.append(bytes);
        buf.append(VerifyUtils::get_verify_code(
            verify_method,
            T::func_code,
            bytes
        ));
        
        return buf;
    }

    template<typename T>
    void write_payload(const T & obj){
        const auto buf = map_payload_to_bytes(verify_method_, obj);
        const auto bytes = buf.to_span();

        phy_.write_bytes(
            nodeid_, 
            T::func_code, 
            bytes
        );
    }

};

}