#include "ObjectDict.hpp"

namespace ymd::canopen{


    
struct Cia301ObjectDict:public StaticObjectDictBase{
    struct ControlWordReg:public RegC32{
        //控制字寄存器 只读32位
        static constexpr Index idx = 0x1000;
        static constexpr SubIndex subidx = 0x00;

        uint16_t protocol_version;
        uint16_t extra_msg;
    };

    struct ErrorReg:public RegC8{
        //错误寄存器 只读8位
        static constexpr Index idx = 0x1001;
        static constexpr SubIndex subidx = 0x0;

        uint8_t uni_err:1;
        uint8_t curr_err:1;
        uint8_t volt_err:1;
        uint8_t temp_err:1;
        uint8_t comm_err:1;
        uint8_t :3;
    };

    struct ManufacturerReg:public RegC32{
        //厂商信息 只读32位
        static constexpr Index idx = 0x1002;
        static constexpr SubIndex subidx = 0x0;

        uint32_t manufacturer_id;
    };

    struct PerdefErrFieldReg:public Reg32{
        //预定义错误域寄存器 可读写32位
    };

    struct CobidSyncMsgReg:public Reg32{
        static constexpr Index idx = 0x1009;
        static constexpr SubIndex subidx = 0x0;

        uint32_t cobid:29;
        uint32_t frame:1;
        uint32_t gen:1;
        uint32_t :1;
    };

    struct CommCyclicPeriodReg:public Reg32{
        static constexpr Index idx = 0x1006;
        static constexpr SubIndex subidx = 0x0;

        uint32_t period;
    };

    struct SyncWindowLengthReg:public Reg32{
        static constexpr Index idx = 0x1007;
        static constexpr SubIndex subidx = 0x0;

        uint32_t length;
    };

    // struct DeviceNameReg : public RegString {
        // // 设备名称寄存器 只读 字符串类型
    //     Index idx = 0x1008;
    //     SubIndex subidx = 0x0;

    //     std::string device_name;
    // };

    // struct HardwareVersionReg:public RegString{
    //     static constexpr Index idx = 0x1009;
    //     static constexpr SubIndex subidx = 0x0;

    //     uint32_t length;
    // };

    struct NodeGuardingPeriodReg : public Reg16 {
        // 节点守护时间寄存器 可读写 16位无符号整数
        static constexpr Index idx = 0x100C;
        static constexpr SubIndex subidx = 0x0;

        uint16_t val;
    };

    struct NodeGuardingPeriodFracReg : public Reg16 {
        // 节点守护时间寄存器 可读写 16位无符号整数
        static constexpr Index idx = 0x100D;
        static constexpr SubIndex subidx = 0x0;

        uint16_t val;
    };


    struct TimeStampReg : public Reg32 {
        static constexpr Index idx = 0x1012;
        static constexpr SubIndex subidx = 0x0;

        uint32_t cobid:29;
        uint32_t frame:1;
        uint32_t produce:1;
        uint32_t consume:1;
    };

    struct GpTimeStampReg : public Reg32 {
        static constexpr Index idx = 0x1014;
        static constexpr SubIndex subidx = 0x0;

        uint32_t cobid:29;
        uint32_t frame:1;
        const uint32_t __resv__:1 = 0;
        uint32_t valid:1;
    };

    struct EmcyDepressTimeReg : public Reg16 {
        static constexpr Index idx = 0x1015;
        static constexpr SubIndex subidx = 0x0;

        uint16_t time;
    };

    struct HeartbeatOverTimeReg : public Reg32 {
        //心跳时间寄存器 可读写 32位无符号整数
        static constexpr Index idx = 0x1016;
        static constexpr SubIndex subidx = 0x0;

        uint16_t time;
        uint8_t node_id;
        uint8_t __resv__;
    };

    ControlWordReg control_word_reg;
    ErrorReg error_reg;
    Cia301ObjectDict() = default;

    std::optional<SubEntry> find(const Didx didx);
};


// class Cia301Device{
// public:
//     Cia301Device(Cia301ObjectDict& od, ) :
//         od_(od), sync_(sync), pdo_(pdo), node_id_(node_id) {}
// private:
//     Cia301ObjectDict od_;
// };


}