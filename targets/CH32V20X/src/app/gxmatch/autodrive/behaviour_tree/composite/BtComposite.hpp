#pragma once

#include "../BtNode.hpp"

namespace btree{

class BtComposite : public BtNode{
public:
    using Node = BtNode;
    using Wrapper = std::shared_ptr<BtNode>;
    using Nodes = std::vector<Wrapper>;

    Nodes children_ = {};
    size_t idx_ = 0;

	BtComposite(const char* name) : BtNode(name){}

    void addChild(BtNode * child){
        children_.push_back(Wrapper(child));
    }

    Wrapper find(const StringView name);
    size_t count() const{ return children_.size();}

    BtNode & operator[] (const size_t idx) override;
    const BtNode & operator[](const size_t idx) const override;
};

}