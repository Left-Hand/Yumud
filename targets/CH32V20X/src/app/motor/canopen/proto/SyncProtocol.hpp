#pragma once

#include "PdoProtocol.hpp"

namespace ymd::canopen{

class SyncProtocol : public Protocol {
public:
    SyncProtocol(Driver& driver, PdoProtocol & pdo, ObjectDictionary& od1)
        : Protocol("Sync", driver, od1), pdo_(pdo) {
    }

    bool processMessage(const CanMsg& msg) override {
        if (!Protocol::processMessage(msg) && (msg.id() != 0x080)) {
            return false;
        }

        // debugPrint("Sync.processMessage()");
        // bool retval = pdo_.sy();

        // notifyListeners(msg);
        return true;
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


};

}