#include "BtDecorator.hpp"

using namespace btree;
using Name = BtDecorator::Name;
using Wrapper = BtDecorator::Wrapper;

BtDecorator::BtDecorator(const Name name, Wrapper child):
    BtNode(name),
    child_(std::move(child)) {
    
}

BtNode & BtDecorator::operator[] (const size_t idx){
    return * child_;
}

const BtNode & BtDecorator::operator[](const size_t idx) const{
    return * child_;
}