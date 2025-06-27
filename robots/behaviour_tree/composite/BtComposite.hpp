#pragma once

#include "../BtNode.hpp"

namespace ymd::btree{

class BtComposite : public BtNode{
public:
    using Child = BtNode;
    // using Wrapper = std::shared_ptr<BtNode>;
    using Wrapper = Child *;
    using Children = std::vector<Wrapper>;

private:
    Children children_ = {};
protected:
    Children & children(){return children_;}
    const Children & children() const{return children_;}
public:
    size_t idx_ = 0;

	BtComposite(const char* name) : BtNode(name){}

    void addChild(Wrapper child){
        children_.push_back(Wrapper(child));
    }

    Wrapper findChild(const StringView name);
    size_t getChildCount() const final override{
        return children_.size();
    }

    StringView getChildPath(const Wrapper child);
    StringView getChildName(const Wrapper child);

    BtNode & operator[] (const size_t idx) override;
    const BtNode & operator[](const size_t idx) const override;
};

}