#include "PdoProtocol.hpp"
#include "PdoSession.hpp"

using namespace ymd::canopen;


PdoProtocol::PdoProtocol(Driver& driver, ObjectDictionary& od1)
    : Protocol("Pdo", driver, od1) {

    // debugPrint("creating Rx Pdo's from Od");
    for (int i = 0; i < 4; i++) {
        // default rx pdo's (RPDO)
        OdEntry & odComm = od1[0x1400 + i].value();
        OdEntry & odMap = od1[0x1600 + i].value();
        PdoSession* pdoSession = new PdoSession(*this, odComm, odMap);
        appendRxPdo(*pdoSession);
    }

    // debugPrint("creating Tx Pdo's from Od");
    for (int i = 0; i < 4; i++) {
        // default tx pdo's (TPDO)
        OdEntry & odComm = od1[0x1800 + i].value();
        OdEntry & odMap = od1[0x1A00 + i].value();
        PdoSession* pdoSession = new PdoSession(*this, odComm, odMap);
        appendTxPdo(*pdoSession);
    }
    // debugPrint("new Pdo");
}


bool PdoProtocol::processMessage(const CanMessage& msg) {
    if (!Protocol::processMessage(msg)) {
        return false;
    }

    auto & ps = (msg.isRemote() == 0) ? rpdos_ : tpdos_;

    for (auto * pdoSession : ps) {
        if (pdoSession->processMessage(msg)) {
            notifyListeners(msg);
            return true;
        }
    }

    return false;
}

bool PdoProtocol::sendSyncEvents() {
    bool retval = false;

    for (auto* ps : tpdos_) {
        if (ps->syncEvent()) {
            retval = true;
        }
    }
    return retval;
}