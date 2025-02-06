#pragma once

// #include "Protocol.hpp"
#include "PdoProtocol.hpp"
// #include "CanMessage.hpp"


namespace ymd::canopen {

class HeartbeatProtocol : public Protocol {
public:
    HeartbeatProtocol(Driver& driver, PdoProtocol & pdo, ObjectDictionary& od1)
        : Protocol("HeartBeat", driver, od1), pdo_(pdo), heartbeatTime(1000), isEnabled(false) {
    }

    bool processMessage(const CanMessage& msg) override {
        if (!Protocol::processMessage(msg) && (msg.id() != 0x700)) {
            return false;
        }

        // debugPrint("HeartbeatProtocol.processMessage()");
        notifyListeners(msg);
        return true;
    }

    bool start() override {
        if (isEnabled) {
            return false;
        }
        isEnabled = true;

        return true;
    }

    bool stop() override {
        if (!isEnabled) {
            return false;
        }
        isEnabled = false;
        return true;
    }

private:
    PdoProtocol & pdo_;
    int heartbeatTime; // in milliseconds
    bool isEnabled;

    void sendHeartbeat() {
		auto & se = getSubEntry(0x1017, 1).unwarp();
        int id = int(se);
        CanMessage msg(
			id, 
			std::make_tuple<uint8_t>(int(se))
		);

        sendMessage(msg);
    }
};

} // namespace ymd::canopen