#include "Sequencer.hpp"

using namespace gxm;


void Sequencer::rotate(Rays & curve, const Ray & from, const Ray & to){
    auto face = from;
    auto delta = limits_.max_gyro / paras_.freq;
    while(face.rad != to.rad){
        STEP_TO(face.rad, to.rad, delta);
        face.rotated(delta);
        curve.push_back(face);
    }
}

void Sequencer::linear(Rays & curve, const Ray & from, const Ray & to){
    auto pdelta = to.org - from.org;

    // this->rotate()
}

template<typename T>
std::optional<ymd::Vector2_t<T>> calculate_circle_center(const ymd::Vector2_t<T> & p1, const ymd::Vector2_t<T> & p2, const T radius){
    return std::nullopt;
}


void Sequencer::circle(Rays & curve, const Ray & from, const Ray & to){
    
}

void Sequencer::fillet(Rays & curve, const Ray & from, const Ray & to){
    
}