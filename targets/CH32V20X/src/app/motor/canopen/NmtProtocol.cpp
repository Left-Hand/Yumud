#include "NmtProtocol.hpp"

using namespace ymd::canopen;

bool NmtProtocol::processMessage(const CanMessage& msg) {
    if (!Protocol::processMessage(msg) && (msg.id() != 0)) {
        return false;
    }

    DEBUG_PRINTLN("NmtProtocol.processMessage()");

    const auto nodeId = msg[1];

    if (nodeId != canOpen->getNodeId()) {
        return false;
    }



    const auto cmd = msg[0];

    using enum NmtCmd;

    switch (NmtCmd(cmd)) {
        case START:
            DEBUG_PRINTLN("Start");
            canOpen->toOperationalState();
            break;
        case STOP:
            DEBUG_PRINTLN("Stop");
            canOpen->toStoppedState();
            break;
        case PREOP:
            DEBUG_PRINTLN("Preop");
            canOpen->toPreoperationalState();
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
    notifyListeners(msg);
    return true;
}