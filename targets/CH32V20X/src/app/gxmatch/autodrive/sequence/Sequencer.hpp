#pragma once

#include "SequenceUtils.hpp"
#include "SequencerBase.hpp"

namespace gxm{

class Sequencer:public SequencerBase{
protected:
    const SequenceLimits & limits_;
    const SequenceParas & paras_;

    Rays curve = {};
public:
    Sequencer(const SequenceLimits & limits, const SequenceParas & paras):
        limits_(limits), paras_(paras){}

    void linear(const Ray & from, const Ray & to);

    void circle(const Ray & from, const Ray & to);

    void fillet(const Ray & from, const Ray & to);

    Rays && fetchCurve(){
        return std::move(curve);
    }
};
}