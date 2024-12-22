#include "BtSelector.hpp"

using namespace ymd::btree;

using Execution = BtNode::Execution;


Execution BtSelector::tick() {
    using enum Execution;

    auto & self = *this;

    while (idx_ < self.getChildCount()) {
        const auto result = self[idx_].tick();
        switch(result){
            case SUCCESS:
                idx_ = 0;
                return SUCCESS;
            case RUNNING:
                return RUNNING;
            default:
                break;
        }
        idx_ += 1;
    }

    idx_ = 0; // Reset index for next run
    return FAILED;
}