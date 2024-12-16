#pragma once

#include "BtDecorator.hpp"

namespace btree {

class Inverter : public BtDecorator {
public:
    explicit Inverter(std::unique_ptr<BtNode> child) : BtDecorator(std::move(child)) {}

    Execution execute() override {
        Execution result = child_->execute();
        switch (result) {
            case Execution::SUCCESS:
                return Execution::FAILED;
            case Execution::FAILED:
                return Execution::SUCCESS;
            case Execution::RUNNING:
                return Execution::RUNNING;
        }
        return Execution::FAILED; // Default case, should not reach here
    }
};

} // namespace btree

#endif // INVERTER_HPP