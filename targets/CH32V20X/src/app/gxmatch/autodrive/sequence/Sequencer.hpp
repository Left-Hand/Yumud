#pragma once

#include "SequenceUtils.hpp"
#include "SequencerBase.hpp"

namespace gxm{

class Sequencer:public SequencerBase{
protected:
    const SequenceLimits & limits_;
    const SequenceParas & paras_;

    Rays curve_ = {};
public:
    Sequencer(const SequenceLimits & limits, const SequenceParas & paras):
        limits_(limits), paras_(paras){}
    
    void rotate(Rays & curve, const Ray & from, const Ray & to);

    void linear(Rays & curve, const Ray & from, const Ray & to);

    void circle(Rays & curve, const Ray & from, const Ray & to);

    void fillet(Rays & curve, const Ray & from, const Ray & to);

    Rays && fetchCurve(){
        return std::move(curve_);
    }
};
}