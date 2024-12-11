#pragma once

#include "../BtNode.hpp"

namespace btree{

class BtComposite : public BtNode{
public:
    using Nodes = std::vector<BtNode*>;

    Nodes children_ = {};
    // size_t idx_ = 0;

	BtComposite(const char* name) : BtNode(name){}



    void addChild(BtNode* child){ children_.push_back(child);}
    BtNode * find(const StringView name);
    size_t count() const{ return children_.size();}

    BtNode & operator[](size_t idx){ 
        auto p_child = children_[idx];

        if(p_child == nullptr){
            HALT;
        }

        return *p_child;
    }
};

}