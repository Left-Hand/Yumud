#pragma once

#include "BtAction.hpp"

namespace ymd::btree{

class BtLambda:public BtNode{
public:
    BtLambda(): BtNode("lambda"){;}
    BtLambda(const Name name): BtNode(name){;}

protected:
    Execution tick() override;
};

}