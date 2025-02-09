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


void NmtMasterProtocol::requestStateSwitch(const uint8_t node_id, const NmtCmd cmd){
    sendMessage(
        CanMsg(
            0x000,
            std::make_tuple<uint8_t, uint8_t>(uint8_t(cmd), uint8_t(node_id))
        )
    );
}


bool NmtSlaveProtocol::processStateSwitchRequest(const CanMsg & msg){
    if (!ProtocolBase::processMessage(msg) && (msg.id() != 0)) {
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



// 任何一个 CANopen 从站上线后，为了提示主站它已经加入网络（便于热插拔），或者
// 避免与其他从站 Node-ID 冲突。这个从站必须发出节点上线报文（boot-up），如图 6.3 所示，
// 节点上线报文的 ID 为 700h+Node-ID，数据为 1 个字节 0。生产者为 CANopen 从站。
void NmtSlaveProtocol::sendBootUp() {
    const auto cobid = 0x700 | dev_.NMT_getNodeId();
    sendMessage(
    CanMsg{
        uint32_t(cobid),
        std::make_tuple<uint8_t>(0)
    });
}

void NmtSlaveProtocol::sendHeartBeat() {
    const auto cobid = 0x700 | dev_.NMT_getNodeId();
    sendMessage(
    CanMsg{
        uint32_t(cobid),
        std::make_tuple<uint8_t>(std::bit_cast<uint8_t>(dev_.NMT_getState()))
    });
}