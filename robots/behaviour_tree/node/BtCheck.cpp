#include "BtCheck.hpp"

using namespace ymd::btree;

BtCheck::Execution BtCheck::tick(){
    bool res = _cb ? _cb() : false;
    return res ? Execution::SUCCESS : Execution::FAILED;
}