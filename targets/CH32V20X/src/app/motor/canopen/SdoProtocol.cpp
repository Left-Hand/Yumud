#include "SdoProtocol.hpp"
#include "SdoSession.hpp"

using namespace ymd::canopen;


bool SdoProtocol::processMessage(const CanMessage & msg) {
    int index = 0;
    int subIndex = 0;
    if (!Protocol::processMessage(msg)) {
        return false;
    }

    if (msg.size() != 8) {
        sendAbort(0, 0, SDOABT_GENERAL_ERROR);
        return false;
    }

    auto it = sessions.find(msg.id());
    if (it == sessions.end()) {
        index = extractIndex(msg);
        subIndex = extractSubIndex(msg);
        SubEntry & se1 = getSubEntry(index, subIndex);
        int txCobId = getSubEntry(0x1200, 2).getInt();
        SdoSession* session = new SdoSession(*this, txCobId, se1);
        sessions[msg.id()] = session;
    }

    SdoSession* session = sessions[msg.id()];
    if (!session->processMessage(msg)) {
        delete session;
        sessions.erase(msg.id());
    }

    notifyListeners(msg);
    return true;
}
