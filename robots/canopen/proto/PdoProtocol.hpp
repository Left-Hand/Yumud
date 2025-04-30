#pragma once


#include "protocolBase.hpp"
#include "../ObjectDict.hpp"
#include "PdoSession.hpp"

namespace ymd::canopen{


// class PdoProtocol : public ProtocolBase {
// public:
//     PdoProtocol(Driver& driver, ObjectDict& od1);

//     bool processMessage(const CanMsg& msg) override;



// private:
//     ObjectDict& od1_;
//     std::array<PdoRxSession, 4> rpdos_ = {
//         PdoRxSession(*this, od1_[0x1400].value(), od1_[0x1600].value()),
//         PdoRxSession(*this, od1_[0x1400].value(), od1_[0x1600].value()),
//         PdoRxSession(*this, od1_[0x1400].value(), od1_[0x1600].value()),
//         PdoRxSession(*this, od1_[0x1400].value(), od1_[0x1600].value())
//     };

//     std::array<PdoTxSession, 4> tpdos_ = {
//         PdoTxSession(*this, od1_[0x1800].value(), od1_[0x1A00].value()),
//         PdoTxSession(*this, od1_[0x1800].value(), od1_[0x1A00].value()),
//         PdoTxSession(*this, od1_[0x1800].value(), od1_[0x1A00].value()),
//         PdoTxSession(*this, od1_[0x1800].value(), od1_[0x1A00].value()),
//     };
// };

}