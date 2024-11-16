#include "Sequencer.hpp"
#include "TrapezoidSolver_t.hpp"

using namespace gxm;
using namespace geometry;
using TrapezoidSolver = TrapezoidSolver_t<real_t>;

void Sequencer::rotate(Curve & curve, const Ray & from, const real_t & end_rad){

    const TrapezoidSolver solver{
        limits_.max_agr, 
        limits_.max_gyr, 
        ABS(end_rad - from.rad)    
    };

    const bool inv = end_rad < from.rad;
    const auto freq = paras_.freq;
    const auto n = size_t(int(solver.period() * freq));
    
    curve.reserve(curve.size() + n);
    
    for(size_t i = 0; i < n; i++){
        const auto t_val = real_t(i) / freq;
        auto rad = solver.forward(t_val);
        curve.push_back(from.rotated(inv ? -rad : rad));
    }
}



void Sequencer::linear(Curve & curve, const Ray & from, const Vector2 & end_pos){
    const auto from_pos = from.org;
    const auto vec = (end_pos - from_pos);
    const auto norm = vec.normalized();
    
    const TrapezoidSolver solver{
        limits_.max_acc, 
        limits_.max_spd, 
        vec.length()
    };

    const auto freq = paras_.freq;
    const auto n = size_t(int(solver.period() * freq));
    
    curve.reserve(curve.size() + n);
    
    for(size_t i = 0; i < n; i++){
        const auto t_val = real_t(i) / freq;
        curve.push_back(Ray(from.org + norm * solver.forward(t_val), from.rad));
    }
}



void Sequencer::arc(Curve & curve, const Ray & from, const Ray & to, const real_t & radius){
    const auto center_opt = calculate_fillet_center(from.normal(), to.normal(), radius);
    if(!center_opt) return;
    // const auto center = center_opt.value();
    
    // TrapezoidSolver solver{
    //     limits_.max_agr, 
    //     limits_.max_gyr, 
    //     ABS(end_rad - from.rad)    
    // };

    // bool inv = end_rad < from.rad;
    // const auto freq = paras_.freq;
    // const auto n = size_t(int(solver.period() * freq));
    
    // curve.reserve(curve.size() + n);
    // for(size_t i = 0; i < n; i++){
    //     const auto t_val = real_t(i) / freq;
    //     auto rad = solver.forward(t_val);
    //     curve.push_back(from.rotated(inv ? -rad : rad));
    // }
}

void Sequencer::fillet(Curve & curve, const Ray & from, const Ray & to){
    auto from_line = from.normal();
    auto to_line = to.normal();

    auto mid_p_opt = from_line.intersection(to_line);
    if(!mid_p_opt) return;

    auto mid_p = mid_p_opt.value();
    
    this->linear(curve, from, mid_p);
    this->rotate(curve, Ray{mid_p, from.rad}, to.rad);
    this->linear(curve, Ray{mid_p, to.rad}, to.org);
}