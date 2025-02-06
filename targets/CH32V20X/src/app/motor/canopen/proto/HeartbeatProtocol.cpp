#include "HeartbeatProtocol.hpp"

using namespace ymd::canopen;

void HeartbeatProtocol::sendHeartbeat() {
    auto & se = getSubEntry(0x1017, 1).unwarp();
    int id = int(se);
    CanMsg msg(
        id, 
        std::make_tuple<uint8_t>(int(se))
    );

    sendMessage(msg);
}