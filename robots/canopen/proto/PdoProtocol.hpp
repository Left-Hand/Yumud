#pragma once


#include "protocolBase.hpp"
#include "../ObjectDict.hpp"
#include "PdoSession.hpp"

namespace ymd::canopen{


class PdoProtocol : public ProtocolBase {
public:
    PdoProtocol(Driver& driver, ObjectDictIntf & od1);

    bool processMessage(const BxCanFrame& msg) override;
private:
    ObjectDictIntf & od1_;
    std::array<PdoRxSession, 4> rpdos_ = {
        PdoRxSession(*this, od1_.get_entry(0x1400).unwrap(), od1_.get_entry(0x1600).unwrap()),
        PdoRxSession(*this, od1_.get_entry(0x1400).unwrap(), od1_.get_entry(0x1600).unwrap()),
        PdoRxSession(*this, od1_.get_entry(0x1400).unwrap(), od1_.get_entry(0x1600).unwrap()),
        PdoRxSession(*this, od1_.get_entry(0x1400).unwrap(), od1_.get_entry(0x1600).unwrap())
    };

    std::array<PdoTxSession, 4> tpdos_ = {
        PdoTxSession(*this, od1_.get_entry(0x1800).unwrap(), od1_.get_entry(0x1A00).unwrap()),
        PdoTxSession(*this, od1_.get_entry(0x1800).unwrap(), od1_.get_entry(0x1A00).unwrap()),
        PdoTxSession(*this, od1_.get_entry(0x1800).unwrap(), od1_.get_entry(0x1A00).unwrap()),
        PdoTxSession(*this, od1_.get_entry(0x1800).unwrap(), od1_.get_entry(0x1A00).unwrap()),
    };
};

}