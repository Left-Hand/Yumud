#pragma once

#include "BtDecorator.hpp"

namespace btree {

class BtRepeat : public BtDecorator {
public:
    using BtDecorator::BtDecorator;

    explicit BtRepeat(Wrapper child):
        BtDecorator("Inverter", std::move(child)) {}

    Execution tick() override;

private:
    size_t repeat_count_;
    size_t current_count_;
};

} // namespace btree
