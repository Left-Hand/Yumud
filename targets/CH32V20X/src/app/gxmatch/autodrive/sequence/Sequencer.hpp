#pragma once

#include "SequenceUtils.hpp"
#include "SequencerBase.hpp"

namespace gxm{

class Sequencer:public SequencerBase{
protected:
    const SequenceLimits & limits_;
    const SequenceParas & paras_;

    void rotate(Curve & curve, const Ray & from, const real_t & end_rad);

    void linear(Curve & curve, const Ray & from, const Vector2 & end_pos);
public:
    Sequencer(const SequenceLimits & limits, const SequenceParas & paras):
        limits_(limits), paras_(paras){}
    

    void arc(Curve & curve, const Ray & from, const Ray & to, const real_t & radius);
    
    void sideways(Curve & curve, const Ray & from, const Ray & to);

    void follow(Curve & curve, const Ray & from, const Ray & to);

    void shift(Curve & curve, const Ray & from, const Ray & to);

    void spin(Curve & curve, const Ray & from, const Ray & to);
};
}