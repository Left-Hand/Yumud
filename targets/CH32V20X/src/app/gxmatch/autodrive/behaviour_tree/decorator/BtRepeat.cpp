#include "BtRepeat.hpp"

using namespace btree;

using Execution = BtNode::Execution;

Execution BtRepeat::tick(){
    auto & self = *this;

    if(current_count_ >= repeat_count_){
        current_count_ = 0;
        return Execution::SUCCESS;
    }

    Execution result = self[0].tick();
    if (result == Execution::FAILED) {
        return Execution::FAILED;
    } else if (result == Execution::SUCCESS) {
        current_count_++;
    }

    return Execution::RUNNING;
}
