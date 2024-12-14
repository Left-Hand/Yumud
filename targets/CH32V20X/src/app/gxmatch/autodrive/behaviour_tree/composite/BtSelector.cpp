#include "BtSelector.hpp"

using namespace btree;

using Execution = BtNode::Execution;


Execution BtSelector::execute() {
    using enum Execution;

    Execution result;
    auto & self = *this;
	if(idx_ < self.count()){
		if (self[idx_].execute() == Execution::SUCCESS){
            idx_ += 1;
        }
    }
	
	// if idx_ >= get_child_count() || result == SUCCEED:
	// 	idx_ = 0
	// 	if result == SUCCEED:
	// 		return SUCCEED
	
	return FAILED;
}