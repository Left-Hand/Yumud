#pragma once

#include "PdoProtocol.hpp"

namespace ymd::canopen{

class SyncProtocol : public Protocol {
public:
    SyncProtocol(Driver& driver, PdoProtocol & pdo, ObjectDictionary& od1)
        : Protocol(driver, "SYNC", od1), pdo_(pdo) {
        // debugPrint("new Sync");
    }

    bool processMessage(const CanMessage& msg) override {
        if (!Protocol::processMessage(msg) && (msg.id() != 0x080)) {
            return false;
        }

        // debugPrint("Sync.processMessage()");
        bool retval = pdo_.sendSyncEvents();

        notifyListeners(msg);
        return retval;
    }

    bool start() override {
        if (Protocol::start()) {
            // debugPrint("sync starting");
            return true;
        }
        return false;
    }

private:
    PdoProtocol & pdo_;

    void sendSync() {
        int id = getSubEntry(0x1005, 1).getInt();
        CanMessage msg(id);
        sendMessage(msg);
    }
};

}