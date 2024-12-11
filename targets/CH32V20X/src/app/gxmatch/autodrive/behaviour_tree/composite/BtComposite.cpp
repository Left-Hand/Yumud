#include "BtComposite.hpp"

using namespace btree;

using Execution = BtNode::Execution;


BtNode * BtComposite::find(const StringView name){
    for(auto & child : children_){
        if(child->name() == name){
            return child;
        }
    }
    return nullptr;
}