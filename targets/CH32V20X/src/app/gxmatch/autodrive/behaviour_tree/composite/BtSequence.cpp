#include "BtSequence.hpp"

using namespace ymd::btree;

using Execution = BtNode::Execution;


// Execution BtSequence::execute() {
//     bool running = false;
//     for (size_t i = 0; i < children_.size(); i++) {
//         auto& child = children_[i];
//         if (child) {
//             switch (child->execute()) {
//                 case Execution::SUCCESS:
//                     // No need to do anything, continue checking other children
//                     break;
//                 case Execution::FAILED:
//                     return Execution::FAILED;
//                 case Execution::RUNNING:
//                     running = true;
//                     break;
//             }
//         }
//     }
//     return running ? Execution::RUNNING : Execution::SUCCESS;
// }

Execution BtSequence::tick() {
    auto & self = *this;
    auto & children = self.children();
    
    bool running = false;
    for (size_t i = 0; i < children.size(); ++i) {
        auto & child = children[i];
        if (!child) {
            // Handle the case where a child node is null
            // This could be an error or a no-op depending on your design
            continue; // Skip this child and move to the next one
        }

        switch (child->tick()) {
            case Execution::SUCCESS:
                // Continue to the next child
                break;
            case Execution::FAILED:
                return Execution::FAILED; // Return FAILED immediately
            case Execution::RUNNING:
                running = true;
                break;
            default:
                break;
        }
    }
    return running ? Execution::RUNNING : Execution::SUCCESS; // Return RUNNING if any child is RUNNING, otherwise SUCCESS
}