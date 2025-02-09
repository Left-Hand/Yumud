#pragma once

#include "Protocol.hpp"
#include "PdoSession.hpp"

namespace ymd::canopen{


class PdoProtocol : public ProtocolBase {
public:
    PdoProtocol(Driver& driver, ObjectDictionary& od1);

    bool processMessage(const CanMsg& msg) override;

private:
    std::array<PdoRxSession, 4> rpdos_ = {
        PdoRxSession(*this, getEntry(0x1400).value(), getEntry(0x1600).value()),
        PdoRxSession(*this, getEntry(0x1400).value(), getEntry(0x1600).value()),
        PdoRxSession(*this, getEntry(0x1400).value(), getEntry(0x1600).value()),
        PdoRxSession(*this, getEntry(0x1400).value(), getEntry(0x1600).value())
    };

    std::array<PdoTxSession, 4> tpdos_ = {
        PdoTxSession(*this, getEntry(0x1800).value(), getEntry(0x1A00).value()),
        PdoTxSession(*this, getEntry(0x1800).value(), getEntry(0x1A00).value()),
        PdoTxSession(*this, getEntry(0x1800).value(), getEntry(0x1A00).value()),
        PdoTxSession(*this, getEntry(0x1800).value(), getEntry(0x1A00).value()),
    };
};

}