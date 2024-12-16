#pragma once

#include "BtDecorator.hpp"

namespace btree {

class Repeater : public BtDecorator {
public:
    Repeater(std::unique_ptr<BtNode> child, size_t repeat_count)
        : BtDecorator(std::move(child)), repeat_count_(repeat_count), current_count_(0) {}

    Execution execute() override {
        if (current_count_ >= repeat_count_) {
            return Execution::SUCCESS;
        }

        Execution result = child_->execute();
        if (result == Execution::FAILED) {
            return Execution::FAILED;
        } else if (result == Execution::SUCCESS) {
            current_count_++;
        }

        return Execution::RUNNING;
    }

private:
    size_t repeat_count_;
    size_t current_count_;
};

} // namespace btree
