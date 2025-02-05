#pragma once

#include "Protocol.hpp"
#include "CanOpenNode.hpp"
#include "sys/debug/debug_inc.h"

namespace ymd::canopen{



class NmtProtocol : public Protocol {
public:
    // NMT commands
    static constexpr int START = 1;
    static constexpr int STOP = 2;
    static constexpr int PREOP = 128;
    static constexpr int RESET_NODE = 129;
    static constexpr int RESET_COMM = 130;

    NmtProtocol(Driver& driver, CanOpenNode * coDevice, ObjectDictionary& od1)
        : Protocol(driver, "NMT", od1), canOpen(coDevice) {
    }

    bool processMessage(const CanMessage& msg) override {
        if (!Protocol::processMessage(msg) && (msg.id() != 0)) {
            return false;
        }

        DEBUG_PRINTLN("NmtProtocol.processMessage()");
        // if (debug) {
        //     msg.dump();
        // }
        int nodeId = (0x000000FF & msg[1]);

        if (nodeId != canOpen->getNodeId()) {
            return false;
        }

        int cmd = (0x000000FF & msg[0]);
        // DEBUG_PRINTLN("cmd :" + std::to_string(cmd) + " (0x" + toIndexFmt(cmd) + ")");
        switch (cmd) {
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
                // std::cout << "ERROR unknown cmd 0x" << toIndexFmt(cmd) << std::endl;
                return false;
        }
        notifyListeners(msg);
        return true;
    }

    bool start() override {
        if (Protocol::start()) {
            DEBUG_PRINTLN("sending boot up message");
            if (!sendBootUp()) {
                DEBUG_PRINTLN("ERROR; starting nmt no nodeid");
                return false;
            }
            return true;
        }
        return false;
    }

private:
    CanOpenNode * canOpen;

    bool sendBootUp() {
        // uint8_t dataBytes[1];
        // dataBytes[0] = static_cast<uint8_t>(0);
        int id = NODE_GUARD << 7 | canOpen->getNodeId();
        // std::cout << "sending bootup message: " << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << id << std::endl;
        CanMessage msg(id, std::make_tuple<uint8_t>(0));
        sendMessage(msg);
        return true;
    }
};


}