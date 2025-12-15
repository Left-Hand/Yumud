#include "PdoProtocol.hpp"
#include "PdoSession.hpp"

using namespace ymd::canopen;

#if 0

PdoProtocol::PdoProtocol(Driver& driver, ObjectDict & od1)
    : ProtocolBase("Pdo", driver) od1_(od1){
}


bool PdoProtocol::processMessage(const BxCanFrame& msg) {
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

#endif