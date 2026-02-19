#pragma once

#include "BtDecorator.hpp"

namespace ymd::btree {

class BtInverter : public BtDecorator {
public:
    using BtDecorator::BtDecorator;

    explicit BtInverter(Wrapper child):
        BtDecorator("Inverter", std::move(child)) {}

    Execution tick() override;
};

} // namespace btree
