#include "NmtProtocol.hpp"

using namespace ymd::canopen;


// bool NmtProtocol::start() {
//     if (Protocol::start()) {
//         DEBUG_PRINTLN("sending boot up message");
//         if (!sendBootUp()) {
//             DEBUG_PRINTLN("ERROR; starting nmt no nodeid");
//             return false;
//         }
//         return true;
//     }
//     return false;
// }

bool NmtSlaveProtocol::processStateSwitchRequest(const CanMsg & msg){
    if (!Protocol::processMessage(msg) && (msg.id() != 0)) {
        return false;
    }

    const auto nodeId = msg[1];

    if (nodeId != dev_.NMT_getNodeId()) {
        return false;
    }

    const auto cmd = msg[0];

    using enum NmtCmd;

    switch (NmtCmd(cmd)) {
        case START:
            DEBUG_PRINTLN("Start");
            dev_.NMT_toOperationalState();
            break;
        case STOP:
            DEBUG_PRINTLN("Stop");
            dev_.NMT_toStoppedState();
            break;
        case PREOP:
            DEBUG_PRINTLN("Preop");
            dev_.NMT_toPreoperationalState();
            break;
        case RESET_NODE:
            DEBUG_PRINTLN("reset node");
            break;
        case RESET_COMM:
            DEBUG_PRINTLN("reset comm");
            break;
        default:
            DEBUG_PRINTLN("unknown comm");
            return false;
    }
    // notifyListeners(msg);
    return true;
}

void NmtMasterProtocol::requestStateSwitch(const uint8_t node_id, const NmtCmd cmd){
    sendMessage(
        CanMsg(
            0x000,
            std::make_tuple<uint8_t, uint8_t>(uint8_t(cmd), uint8_t(node_id))
        )
    );
}