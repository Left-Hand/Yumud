#include "Sequencer.hpp"
#include "TrapezoidSolver_t.hpp"

using namespace gxm;
using TrapezoidSolver = TrapezoidSolver_t<real_t>;

void Sequencer::rotate(Rays & curve, const Ray & from, const real_t & end_rad){

    TrapezoidSolver solver{
        limits_.max_angular, 
        limits_.max_gyro, 
        ABS(end_rad - from.rad)    
    };

    bool inv = end_rad < from.rad;
    const auto freq = paras_.freq;
    const auto n = solver.period() * freq;
    
    auto t_val = real_t(0);
    const auto t_delta = real_t(1) / freq; 
    for(size_t i = 0; i < n; i++){
        auto rad = solver.forward(t_val);
        curve.push_back(from.rotated(inv ? -rad : rad));
        t_val += t_delta;
    }
}



void Sequencer::linear(Rays & curve, const Ray & from, const Vector2 & end_pos){
    const auto from_pos = from.org;
    const auto vec = (end_pos - from_pos);
    const auto norm = vec.normalized();
    
    TrapezoidSolver solver{
        limits_.max_acc, 
        limits_.max_spd, 
        vec.length()
    };

    const auto freq = paras_.freq;
    const auto n = solver.period() * freq;
    
    auto t_val = real_t(0);
    const auto t_delta = real_t(1) / freq; 
    for(size_t i = 0; i < n; i++){
        curve.push_back(Ray(from.org + norm * solver.forward(t_val), from.rad));
        t_val += t_delta;
    }
}



void Sequencer::circle(Rays & curve, const Ray & from, const Ray & to){
    // auto from_line = from.normal();
    // auto to_line = to.normal();

    // auto mid_p_opt = from_line.intersection(to_line);
    // if(!mid_p_opt) return;

    // auto mid_p = mid_p_opt.value();
    
    // this->linear(curve, from, mid_p);
    // this->rotate(curve, Ray{mid_p, from.rad}, to.rad);
    // this->linear(curve, Ray{mid_p, to.rad}, to.org);
}

void Sequencer::fillet(Rays & curve, const Ray & from, const Ray & to){
    auto from_line = from.normal();
    auto to_line = to.normal();

    auto mid_p_opt = from_line.intersection(to_line);
    if(!mid_p_opt) return;

    auto mid_p = mid_p_opt.value();
    
    this->linear(curve, from, mid_p);
    this->rotate(curve, Ray{mid_p, from.rad}, to.rad);
    this->linear(curve, Ray{mid_p, to.rad}, to.org);
}