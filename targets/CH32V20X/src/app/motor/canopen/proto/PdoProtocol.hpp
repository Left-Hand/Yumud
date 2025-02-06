#pragma once

#include "Protocol.hpp"
#include "PdoSession.hpp"

namespace ymd::canopen{


class PdoProtocol : public Protocol {
public:
    PdoProtocol(Driver& driver, ObjectDictionary& od1);

    bool processMessage(const CanMsg& msg) override;

private:
    std::array<PdoRxSession, 4> rpdos_ = {
        PdoRxSession(*this, getEntry(0x1400).unwarp(), getEntry(0x1600).unwarp()),
        PdoRxSession(*this, getEntry(0x1400).unwarp(), getEntry(0x1600).unwarp()),
        PdoRxSession(*this, getEntry(0x1400).unwarp(), getEntry(0x1600).unwarp()),
        PdoRxSession(*this, getEntry(0x1400).unwarp(), getEntry(0x1600).unwarp())
    };

    std::array<PdoTxSession, 4> tpdos_ = {
        PdoTxSession(*this, getEntry(0x1800).unwarp(), getEntry(0x1A00).unwarp()),
        PdoTxSession(*this, getEntry(0x1800).unwarp(), getEntry(0x1A00).unwarp()),
        PdoTxSession(*this, getEntry(0x1800).unwarp(), getEntry(0x1A00).unwarp()),
        PdoTxSession(*this, getEntry(0x1800).unwarp(), getEntry(0x1A00).unwarp()),
    };
};

}