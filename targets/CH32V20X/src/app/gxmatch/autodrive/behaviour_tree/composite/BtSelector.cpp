#include "BtSelector.hpp"

using namespace btree;

using Execution = BtNode::Execution;


Execution BtSelector::execute() {
    using enum Execution;
    // auto & self = *this;
	// while (idx_ < count()):
	// 	result = get_child(idx_)._task()
	// 	if result == FAILED:
	// 		idx_ += 1
	// 	else:
	// 		break
	
	// if idx_ >= get_child_count() || result == SUCCEED:
	// 	idx_ = 0
	// 	if result == SUCCEED:
	// 		return SUCCEED
	
	return FAILED;
}