#pragma once

#include "SequenceUtils.hpp"

namespace gxm{

class Sequencer{
protected:
    const SequenceLimits & limits_;
    const SequenceParas & paras_;

    Rays curve = {};
public:
    Sequencer(const SequenceLimits & limits, const SequenceParas & paras):
        limits_(limits), paras_(paras){}

    void linear();

    void circle();

    void fillet();
};
}