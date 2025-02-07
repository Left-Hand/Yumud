#include "PdoProtocol.hpp"
#include "PdoSession.hpp"

using namespace ymd::canopen;


PdoProtocol::PdoProtocol(Driver& driver, ObjectDictionary& od1)
    : Protocol("Pdo", driver, od1) {
}


bool PdoProtocol::processMessage(const CanMsg& msg) {
    // auto & ps = (msg.isRemote() == 0) ? rpdos_ : tpdos_;

    // for (auto & pdoSession : ps) {
    //     if (pdoSession->processMessage(msg)) {
    //         return true;
    //     }
    // }

    return Protocol::processMessage(msg);
}

// bool PdoProtocol::sendSyncEvents() {
//     bool retval = false;

//     for (auto & ps : tpdos_) {
//         if (ps->onSyncEvent()) {
//             retval = true;
//         }
//     }

//     return retval;
// }