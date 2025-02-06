#pragma once

#include "Protocol.hpp"
// #include "PdoSession.hpp"

namespace ymd::canopen{

class PdoSession;

class PdoProtocol : public Protocol {
public:
    PdoProtocol(Driver& driver, ObjectDictionary& od1);

    // bool sendSyncEvents();

    bool processMessage(const CanMsg& msg) override;

private:
    std::vector<std::unique_ptr<PdoSession>> rpdos_;
    std::vector<std::unique_ptr<PdoSession>> tpdos_;
};



class TxPdoProtocol: public Protocol {

};

class RxPdoProtocol: public Protocol {

};

}