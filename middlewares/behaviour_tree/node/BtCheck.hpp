#pragma once

#include "../BtNode.hpp"
#include <functional>

namespace ymd::btree{

class BtCheck : public BtNode{
public:
    using Callback = std::function<bool(void)>; 
protected:
    Callback _cb;


public:
    using BtNode::BtNode;

    BtCheck(const Name name, auto && cb): 
        BtNode(name),
        _cb(std::forward(cb)){;}

    BtCheck(auto && cb):
        BtCheck("check", cb){;}

    Execution tick() override;
};

}