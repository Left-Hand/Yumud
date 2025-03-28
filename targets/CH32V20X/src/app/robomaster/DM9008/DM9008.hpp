#pragma once

#include "../utils.hpp"


namespace ymd::rmst{

struct DM9008_Fault{

    enum Kind:uint8_t{
        // �������·����������¶ȣ�120�棬���µ�����˳�
        // ��ʹ��ģʽ��
        // ������·���������ʹ�������趨�����鲻����
        // 100�棬���µ�����˳���ʹ��ģʽ��
        // �����ѹ����������ʹ�������趨�����鲻����52V��
        // ��ѹ���˳���ʹ��ģʽ��
        // ͨѶ��ʧ�������趨������û���յ�CANָ��Զ�
        // �˳���ʹ��ģʽ��
        // �����������������ʹ�������趨�����鲻����39A��
        // �������˳���ʹ��ģʽ��
        // ���Ƿѹ����������Դ��ѹ�����趨ֵ�����˳���ʹ
        // ��ģʽ������Դ��ѹ������15

        
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