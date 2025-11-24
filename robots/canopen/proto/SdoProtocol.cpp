#include "SdoProtocol.hpp"
#include "SdoSession.hpp"

using namespace ymd::canopen;


bool SdoProtocol::processMessage(const CanClassicFrame & frame) {
    // int index = 0;
    // int subIndex = 0;
    if (!ProtocolBase::processMessage(msg)) {
        return false;
    }

    if (msg.length() != 8) {
        send_abort(0, 0, SdoAbortCode::GeneralError);
        return false;
    }

    // auto it = sessions.find(msg.id());
    // if (it == sessions.end()) {
    //     index = extractIndex(msg);
    //     subIndex = extractSubIndex(msg);
    //     SubEntry & se1 = getSubEntry(index, subIndex).value();
    //     int txCobId = int(getSubEntry(0x1200, 2).value());
    //     SdoSession* session = new SdoSession(*this, txCobId, se1);
    //     sessions[msg.id()] = session;
    // }

    // SdoSession* session = sessions[msg.id()];
    // if (!session->processMessage(msg)) {
    //     delete session;
    //     sessions.erase(msg.id());
    // }

    // notifyListeners(msg);
    return true;
}
