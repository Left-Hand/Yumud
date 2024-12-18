#include "BtComposite.hpp"

using namespace btree;

using Execution = BtNode::Execution;
using Wrapper = BtComposite::Wrapper;

Wrapper BtComposite::find(const StringView name){
    for(auto & child : children_){
        if(child->name() == name){
            return child;
        }
    }
    return nullptr;
}

BtNode & BtComposite::operator[](size_t idx){
    if(idx >= children_.size()){
        HALT;
    }

    auto p_child = children_[idx];

    if(p_child == nullptr){
        HALT;
    }

    return *p_child;
}

const BtNode & BtComposite::operator[](size_t idx) const {
    if(idx >= children_.size()){
        HALT;
    }

    auto p_child = children_[idx];

    if(p_child == nullptr){
        HALT;
    }

    return *p_child;
}

