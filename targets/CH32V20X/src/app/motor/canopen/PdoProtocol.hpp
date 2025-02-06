#pragma once

#include "Protocol.hpp"
// #include "PdoSession.hpp"

namespace ymd::canopen{

class PdoSession;

class PdoProtocol : public Protocol {
public:
    PdoProtocol(Driver& driver, ObjectDictionary& od1);


    bool sendSyncEvents();

    bool processMessage(const CanMessage& msg) override;

private:
    std::vector<std::unique_ptr<PdoSession>> rpdos_;
    std::vector<std::unique_ptr<PdoSession>> tpdos_;

    // void appendRxPdo(std::unique_ptr<PdoSession> ps) {
    //     rpdos_.push_back(ps);
    // }

    // void appendTxPdo(std::unique_ptr<PdoSession> ps) {
    //     tpdos_.push_back(ps);
    // }

};

}