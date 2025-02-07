#pragma once

#include "Protocol.hpp"
#include "canopen/CanOpenNode.hpp"
#include "sys/debug/debug_inc.h"

namespace ymd::canopen{


// 04h为停止状态，05h为操作状态，7Fh为预操作状
// 态
enum class NmtState : uint8_t {
    Stopped = 0x04,
    PreOperational = 0x7F,
    Operational = 0x05
};


class NmtDealerIntf{
public:
    virtual int NMT_getNodeId() = 0;
    virtual NmtState NMT_getState() = 0;
    virtual void NMT_toOperationalState() = 0;
    virtual void NMT_toStoppedState() = 0;
    virtual void NMT_toPreoperationalState() = 0;
};

class NmtProtocolBase : public Protocol {
public:
    // NMT commands

    enum class NmtCmd{
        START = 1,
        STOP = 2,
        PREOP = 128,
        RESET_NODE = 129,
        RESET_COMM = 130
    };

    NmtProtocolBase(const StringView name, Driver& driver, ObjectDictionary& od1, NmtDealerIntf & coDevice)
        : Protocol(name, driver, od1), dev_(coDevice) {
    }

    bool processMessage(const CanMsg& msg) override;

    bool start() override ;

// private:
protected:
    NmtDealerIntf & dev_;
};

class NmtMasterProtocol : public NmtProtocolBase {
public:
    NmtMasterProtocol(NmtDealerIntf & dev, Driver& driver, ObjectDictionary& od1, NmtDealerIntf & coDevice)
        : NmtProtocolBase("nmaster",driver, od1, coDevice) {}

    void requestStateSwitch(const uint8_t node_id, const NmtCmd cmd);
};

class NmtSlaveProtocol : public NmtProtocolBase {
public:
    NmtSlaveProtocol(NmtDealerIntf & dev, Driver& driver, ObjectDictionary& od1, NmtDealerIntf & coDevice)
        : NmtProtocolBase("nslave",driver, od1, coDevice){}

    // 任何一个 CANopen 从站上线后，为了提示主站它已经加入网络（便于热插拔），或者
    // 避免与其他从站 Node-ID 冲突。这个从站必须发出节点上线报文（boot-up），如图 6.3 所示，
    // 节点上线报文的 ID 为 700h+Node-ID，数据为 1 个字节 0。生产者为 CANopen 从站。
    void sendBootUp() {
        const auto cobid = 0x700 | dev_.NMT_getNodeId();
        sendMessage(
        CanMsg{
            uint32_t(cobid),
            std::make_tuple<uint8_t>(0)
        });
    }

    void sendHeartBeat() {
        const auto cobid = 0x700 | dev_.NMT_getNodeId();
        sendMessage(
        CanMsg{
            uint32_t(cobid),
            std::make_tuple<uint8_t>(std::bit_cast<uint8_t>(dev_.NMT_getState()))
        });
    }

    bool processStateSwitchRequest(const CanMsg & msg);
};


}