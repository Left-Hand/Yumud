#include "BtParallel.hpp"

using namespace btree;

using Execution = BtNode::Execution;



// Execution BtParallel::execute() {
//     bool running = true;
//     for (size_t i = 0; i < children_.size(); i++) {
//         auto& child = children_[i];
//         if (child) {
//             switch (child->execute()) {
//                 case Execution::SUCCESS:
//                     break;
//                 case Execution::FAILED:
//                     return Execution::FAILED;
//                     break;
//                 case Execution::RUNNING:
//                     running = true;
//                     break;
//             }
//         }
//     }
//     return running ? Execution::RUNNING : Execution::SUCCESS;
// }

Execution BtParallel::tick() {
    auto & self = *this;
    auto & children = self.children();

    bool running = false;

    for (size_t i = 0; i < children.size(); i++) {
        auto child = children[i];
        if (child) {
            switch (child->tick()) {
                case Execution::SUCCESS:
                    // No need to do anything, continue checking other children
                    break;
                case Execution::FAILED:
                    return Execution::FAILED;
                case Execution::RUNNING:
                    running = true;
                    break;
            }
        }
    }
    return running ? Execution::RUNNING : Execution::SUCCESS;
}