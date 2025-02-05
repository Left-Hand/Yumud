#pragma once

// #include "Protocol.hpp"
#include "PdoProtocol.hpp"
// #include "CanMessage.hpp"


namespace ymd::canopen {

class HeartbeatProtocol : public Protocol {
public:
    HeartbeatProtocol(Driver& driver, PdoProtocol & pdo, ObjectDictionary& od1)
        : Protocol(driver, "HEARTBEAT", od1), pdo_(pdo), heartbeatTime(1000), isEnabled(false) {
        debugPrint("new HeartbeatProtocol");
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
        debugPrint("heartbeat starting");

        // while (isEnabled) {
        //     std::this_thread::sleep_for(std::chrono::milliseconds(heartbeatTime));
        //     sendHeartbeat();
        // }
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
        std::vector<uint8_t> dataBytes(1);
        dataBytes[0] = static_cast<uint8_t>(getSubEntry(0x1017, 1).getInt());
        int id = getSubEntry(0x1017, 0).getInt();
        CanMessage msg(id, dataBytes.data(), dataBytes.size());
        sendMessage(msg);
    }

    void debugPrint(const std::string& v) const {
        // if (debug) {
        //     std::cout << v << std::endl;
        // }
    }
};

} // namespace ymd::canopen