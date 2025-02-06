#pragma once

#include "Protocol.hpp"
// #include "PdoSession.hpp"

namespace ymd::canopen{

class PdoSession;

class PdoProtocol : public Protocol {
public:
    PdoProtocol(Driver& driver, ObjectDictionary& od1);

    void appendRxPdo(PdoSession & ps) {
        rpdos_.push_back(&ps);
    }

    void appendTxPdo(PdoSession & ps) {
        tpdos_.push_back(&ps);
    }

    bool sendSyncEvents();

    bool processMessage(const CanMessage& msg) override;

private:
    std::vector<PdoSession*> rpdos_;
    std::vector<PdoSession*> tpdos_;
};

}