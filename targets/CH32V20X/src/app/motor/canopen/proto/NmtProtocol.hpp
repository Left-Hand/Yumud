#pragma once

#include "Protocol.hpp"
#include "canopen/CanOpenNode.hpp"
#include "sys/debug/debug_inc.h"

namespace ymd::canopen{



class NmtProtocol : public Protocol {
public:
    // NMT commands

    enum class NmtCmd{
        START = 1,
        STOP = 2,
        PREOP = 128,
        RESET_NODE = 129,
        RESET_COMM = 130
    };

    NmtProtocol(Driver& driver, CanOpenNode * coDevice, ObjectDictionary& od1)
        : Protocol("NMT", driver, od1), canOpen(coDevice) {
    }

    bool processMessage(const CanMsg& msg) override;

    bool start() override ;

private:
    CanOpenNode * canOpen;

    bool sendBootUp() {
        sendMessage(CanMsg{
            uint32_t(NODE_GUARD << 7 | canOpen->getNodeId()),
            std::make_tuple<uint8_t>(0)});
        return true;
    }
};


}