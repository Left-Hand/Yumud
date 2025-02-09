#pragma once

#include "ProtocolBase.hpp"


namespace ymd::canopen{


enum class NmtState : uint8_t {
    Stopped = 0x04,
    PreOperational = 0x7F,
    Operational = 0x05
};

class NmtMasterIntf{
public:
    virtual NmtState NMT_getState(const uint8_t nodeid) = 0;
    virtual void NMT_toOperationalState(const uint8_t nodeid) = 0;
    virtual void NMT_toStoppedState(const uint8_t nodeid) = 0;
    virtual void NMT_toPreoperationalState(const uint8_t nodeid) = 0;
};

class NmtSlaveIntf{
public:
    virtual int NMT_getNodeId() = 0;
    virtual NmtState NMT_getState() = 0;
    virtual void NMT_toOperationalState() = 0;
    virtual void NMT_toStoppedState() = 0;
    virtual void NMT_toPreoperationalState() = 0;
};

class NmtProtocolBase : public ProtocolBase {
protected:
    NmtProtocolBase(const StringView name, Driver & driver)
    : ProtocolBase("Nmt", driver) {
    }

public:
    enum class NmtCmd{
        START = 1,
        STOP = 2,
        PREOP = 128,
        RESET_NODE = 129,
        RESET_COMM = 130
    };

    bool processMessage(const CanMsg& msg) override;

};

class NmtMasterProtocol : public NmtProtocolBase {
protected:
    NmtMasterIntf & dev_;
public:
    NmtMasterProtocol(NmtMasterIntf & dev, Driver& driver)
        : NmtProtocolBase("nmaster",driver ), dev_(dev){}

    // void sendSync() {
    //     int id = int(getSubEntry(0x1005, 1).value());
    //     CanMsg msg(id);
    //     sendMessage(msg);
    // }

    void requestStateSwitch(const uint8_t node_id, const NmtCmd cmd);
};

class NmtSlaveProtocol : public NmtProtocolBase {
protected:
    NmtSlaveIntf & dev_;
public:
    NmtSlaveProtocol(NmtSlaveIntf & dev, Driver& driver)
        : NmtProtocolBase("nslave",driver), dev_(dev){}

    void sendBootUp();
    void sendHeartBeat();
    bool processStateSwitchRequest(const CanMsg & msg);
};


}