#pragma once

#include "../utils.hpp"
#include "core/math/iq/fixed_t.hpp"
#include "primitive/can/bxcan_frame.hpp"


namespace ymd::robots::damiao::dm9008{
using namespace literals;
struct [[nodiscard]] DM9008_Fault{

    enum class [[nodiscard]] Kind:uint8_t{
        // 驱动过温防护，防护温度：120℃，过温电机将退出
        // “使能模式”
        // 电机过温防护，根据使用需求设定，建议不超过
        // 100℃，过温电机将退出“使能模式”
        // 电机过压防护，根据使用需求设定，建议不超过52V，
        // 过压将退出“使能模式”
        // 通讯丢失防护，设定周期内没有收到CAN指令将自动
        // 退出“使能模式”
        // 电机过流防护，根据使用需求设定，建议不超过39A，
        // 过流将退出“使能模式”
        // 电机欠压防护，若电源电压低于设定值，则退出“使
        // 能模式”，电源电压不低于15

        OverVoltage = 8,
        UnderVolage = 9,
        OverCurrent = 10,
        MosOverTemp = 11,
        CoilOverTemp = 12,
        CommLost = 13,
        Overload = 14
    };

    constexpr DM9008_Fault(const Kind kind):
        kind_(kind){}
private:
    Kind kind_;
};

static_assert(sizeof(DM9008_Fault) == 1);

// struct SpdCtrlParams{
//     iq16 x2;
//     void fill_bytes(std::span<uint8_t, 4> bytes) const{

//     }
// };
// struct PosSpdCtrlParams{
//     iq16 x1;
//     iq16 x2;
// };

class [[nodiscard]] DM9008{
public:


    // struct MitCtrlParams{
    //     iq16 p_des;
    //     iq16 v_des;
    //     iq16 kp;
    //     iq16 kd;
    //     iq16 t_ff;
    // };
    using NodeId = uint8_t;

    struct FrameFactory{
        // static constexpr hal::BxCanFrame make_spd_frame(
        //     const NodeId id, 
        //     const SpdCtrlParams & p
        // ){
        //     return hal::BxCanFrame::from_parts(
        //         hal::CanStdId::from_u11(0x100 | id), 
        //         hal::BxCanPayload::from_bytes(
        //             std::bit_cast<std::array<uint8_t, 4>>(float(p.x2))
        //         )
        //     );
        // }

        // static constexpr hal::BxCanFrame make_posspd_frame(
        //     const NodeId id, 
        //     const PosSpdCtrlParams & p
        // ){
        //     return hal::BxCanFrame::from_parts(
        //         hal::CanStdId::from_u11(0x100 | id), 
        //         hal::BxCanPayload::from_bytes(
        //             std::bit_cast<std::array<uint8_t, 8>>(p)
        //         )
        //     );
        // }

    };
};

}