#pragma once

#include "sys/string/string.hpp"
#include "sys/string/StringView.hpp"

namespace btree{


struct BtNode{
public:
    enum Execution{
        SUCCESS, 
        FAILED,
        RUNNING
    };

    using String = ymd::String;
    using StringView = ymd::StringView;
private:
    String name_;
public:
//     T data;
//     BtNode* left;
//     BtNode* right;

//     BtNode(T data){
//         this->data = data;
//         left = nullptr;
//         right = nullptr;
//     }
// };

    BtNode(const StringView name):
        name_(name){;}

    BtNode(const BtNode & other) = default;
    BtNode(BtNode && other) = default;
    virtual ~BtNode() = default;

    virtual Execution execute() = 0;

    StringView name() const{ return StringView(name_); }

};

}