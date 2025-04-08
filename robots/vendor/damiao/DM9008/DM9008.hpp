#pragma once

#include "../utils.hpp"


namespace ymd::rmst{

struct DM9008_Fault{

    enum Kind:uint8_t{
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

    DM9008_Fault(const Kind kind):kind_(kind){}
private:
    Kind kind_;
};static_assert(sizeof(CyberGear_Fault) == 2);

class DM9008{
public:
    struct SpdCtrlParams{real_t spd;};
    struct PosSpdCtrlParams{real_t pos, spd;};

    struct MitCtrlParams{
        real_t p_des;
        real_t v_des;
        real_t kp;
        real_t kd;
        real_t t_ff;
    };
    using NodeId = uint8_t;

    struct FrameFactory{
        static constexpr 
        make_spd_frame(const NodeId id, const SpdCtrlParams & p){
            return hal::CanMsg::from_tuple(CanStdId(0x100 | id), std::make_tuple(
                float(p.spd)
            ));
        }

        static constexpr
        make_posspd_frame(const NodeId id, const PosSpdCtrlParams & p){
            return hal::CanMsg::from_tuple(CanStdId(0x200 | id), std::make_tuple(
                float(p.pos),
                float(p.spd),
            ));
        }

        static constexpr 
        make_mit_frame(const NodeId id, const MitCtrlParams & p){
            std::array<uint8_t, 8> buf;
        }
    };
};

}