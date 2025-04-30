#pragma once

#include "../BtNode.hpp"

namespace ymd::btree {

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

    size_t getChildCount() const final override{
        return 1;
    }

    BtNode & operator[] (const size_t idx) override;
    const BtNode & operator[](const size_t idx) const override;

};

}