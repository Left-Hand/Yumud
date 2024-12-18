#include "BtInverter.hpp"

using namespace btree;

using Execution = BtNode::Execution;

Execution BtInverter::tick(){
    auto & self = *this;
    auto result = self[0].tick();
    switch (result) {
        case Execution::SUCCESS:
            return Execution::FAILED;
        case Execution::FAILED:
            return Execution::SUCCESS;
        default:
            return result;
    }
}