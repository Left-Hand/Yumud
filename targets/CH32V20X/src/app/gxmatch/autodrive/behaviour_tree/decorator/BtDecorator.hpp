#pragma once

#include "../BtNode.hpp"

namespace btree {

class BtDecorator : public BtNode {
public:
    using Child = BtNode;
    using Wrapper = Child *;

private:
    Wrapper child_;

protected:
public:
    explicit BtDecorator(const Name name, Wrapper child);
    explicit BtDecorator(Wrapper child):
        BtDecorator("Decorator", std::move(child)) {}
    virtual ~BtDecorator() = default;

    BtNode & operator[] (const size_t idx);
    const BtNode & operator[](const size_t idx) const ;

};

}