#pragma once

#include "../BtNode.hpp"

namespace ymd::btree{

class BtAction : public BtNode{
public:
    BtAction(): BtNode("action"){;}
    BtAction(const Name name): BtNode(name){;}

    using BtNode::BtNode;
};

}