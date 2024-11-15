#pragma once

#include "SequenceUtils.hpp"
#include "SequencerBase.hpp"

namespace gxm{

class Sequencer:public SequencerBase{
protected:
    const SequenceLimits & limits_;
    const SequenceParas & paras_;

public:
    Sequencer(const SequenceLimits & limits, const SequenceParas & paras):
        limits_(limits), paras_(paras){}
    
    void rotate(Rays & curve, const Ray & from, const real_t & end_rad);

    void linear(Rays & curve, const Ray & from, const Vector2 & end_pos);

    void circle(Rays & curve, const Ray & from, const Ray & to);

    void fillet(Rays & curve, const Ray & from, const Ray & to);
};
}