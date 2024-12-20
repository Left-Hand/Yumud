#pragma once

#include "BtDecorator.hpp"

namespace btree {

class BtRepeat : public BtDecorator {
public:
    using BtDecorator::BtDecorator;
    // const int _repeat_count = 3;

    // explicit BtRepeat(Wrapper child, const int repeat_count):
        // BtDecorator("Inverter", std::move(child)) {}

    explicit BtRepeat(Wrapper child, const size_t repeat_count):
        BtDecorator("Inverter", std::move(child)),
        repeat_count_(repeat_count){}

    Execution tick() override;

private:
    size_t repeat_count_;
    size_t current_count_;
};

} // namespace btree
