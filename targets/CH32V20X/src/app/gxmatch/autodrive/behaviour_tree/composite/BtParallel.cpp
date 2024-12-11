#include "BtParallel.hpp"

using namespace btree;

using Execution = BtNode::Execution;



Execution BtParallel::execute() {
    bool running = true;
    for (size_t i = 0; i < children_.size(); i++) {
        auto& child = children_[i];
        if (child) {
            switch (child->execute()) {
                case Execution::SUCCESS:
                    break;
                case Execution::FAILED:
                    return Execution::FAILED;
                    break;
                case Execution::RUNNING:
                    running = true;
                    break;
            }
        }
    }
    return running ? Execution::RUNNING : Execution::SUCCESS;
}