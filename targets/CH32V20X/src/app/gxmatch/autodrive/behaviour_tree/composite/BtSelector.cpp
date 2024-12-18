#include "BtSelector.hpp"

using namespace btree;

using Execution = BtNode::Execution;


Execution BtSelector::tick() {
    using enum Execution;

    Execution result;
    auto & self = *this;

    while (idx_ < self.count()) {
        result = self[idx_].tick();
        if (result == SUCCESS) {
            idx_ = 0; // Reset index for next run
            return SUCCESS;
        } else if (result == RUNNING) {
            return RUNNING;
        }
        idx_ += 1;
    }

    idx_ = 0; // Reset index for next run
    return FAILED;
}