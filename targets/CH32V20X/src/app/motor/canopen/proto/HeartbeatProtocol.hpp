#pragma once

#include "PdoProtocol.hpp"


namespace ymd::canopen {

class HeartbeatProtocol : public Protocol {
public:
    HeartbeatProtocol(Driver& driver, PdoProtocol & pdo, ObjectDictionary& od)
        : Protocol("HeartBeat", driver, od), pdo_(pdo), heartbeatTime(1000), isEnabled(false) {
    }

    bool processMessage(const CanMsg& msg) override {
        if (!Protocol::processMessage(msg) && (msg.id() != 0x700)) {
            return false;
        }
        return true;
    }

private:
    PdoProtocol & pdo_;
    int heartbeatTime; // in milliseconds
    bool isEnabled;

    void sendHeartbeat();
};

} // namespace ymd::canopen