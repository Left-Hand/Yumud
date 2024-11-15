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
    
    void rotate(Curve & curve, const Ray & from, const real_t & end_rad);

    void linear(Curve & curve, const Ray & from, const Vector2 & end_pos);

    void arc(Curve & curve, const Ray & from, const Ray & to, const real_t & radius);

    void fillet(Curve & curve, const Ray & from, const Ray & to);
};
}