#pragma once

#include "../BtNode.hpp"

namespace btree {

class BtDecorator : public BtNode {
public:
    explicit BtDecorator(std::unique_ptr<BtNode> child) : child_(std::move(child)) {}

    virtual ~BtDecorator() = default;

protected:
    std::unique_ptr<BtNode> child_;
};

}