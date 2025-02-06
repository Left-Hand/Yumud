#include "PdoProtocol.hpp"
#include "PdoSession.hpp"

using namespace ymd::canopen;


PdoProtocol::PdoProtocol(Driver& driver, ObjectDictionary& od1)
    : Protocol("Pdo", driver, od1) {

    auto & self = *this;

    //创建四个Pdo的接收会话
    for (size_t i = 0; i < 4; i++) {
        auto & odComm = od1[0x1400 + i].unwarp();
        auto & odMap = od1[0x1600 + i].unwarp();
        rpdos_.push_back(std::make_unique<PdoSession>(self, odComm, odMap));
    }

    //创建四个Pdo的发送会话
    for (size_t i = 0; i < 4; i++) {
        auto & odComm = od1[0x1800 + i].unwarp();
        auto & odMap = od1[0x1A00 + i].unwarp();
        tpdos_.push_back(std::make_unique<PdoSession>(self, odComm, odMap));
    }
}


bool PdoProtocol::processMessage(const CanMsg& msg) {
    auto & ps = (msg.isRemote() == 0) ? rpdos_ : tpdos_;

    for (auto & pdoSession : ps) {
        if (pdoSession->processMessage(msg)) {
            return true;
        }
    }

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