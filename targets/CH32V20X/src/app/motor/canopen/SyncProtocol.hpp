#pragma once

#include "PdoProtocol.hpp"

namespace ymd::canopen{

class SyncProtocol : public Protocol {
public:
    SyncProtocol(Driver& driver, PdoProtocol & pdo, ObjectDictionary& od1)
        : Protocol("Sync", driver, od1), pdo_(pdo) {
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
        int id = int(getSubEntry(0x1005, 1).unwarp());
        CanMessage msg(id);
        sendMessage(msg);
    }
};

}