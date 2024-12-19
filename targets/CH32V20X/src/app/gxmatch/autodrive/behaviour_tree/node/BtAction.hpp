#pragma once

namespace btree{

class BtAction : public BtNode{
public:
    // BtAction(const char* name) : BtNode(name)
    using BtDecorator::BtNode;
    virtual Execution tick() = 0
};

}