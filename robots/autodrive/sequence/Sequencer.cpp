#include "Sequencer.hpp"
#include "TrapezoidSolver_t.hpp"

using namespace ymd;
using namespace ymd::geometry;
using namespace ymd::robots;

using TrapezoidSolver = TrapezoidSolver_t<real_t>;

void Sequencer::rotate(Curve & curve, const Ray2_t<real_t> & from, const real_t & end_rad){

    const TrapezoidSolver solver{
        limits_.max_agr, 
        limits_.max_gyr, 
        ABS(end_rad - from.rad)    
    };

    const bool inv = end_rad < from.rad;
    const auto freq = paras_.freq;
    const auto n = size_t(solver.period() * freq);
    
    curve.reserve(curve.size() + n);
    
    for(size_t i = 0; i < n; i++){
        const auto t_val = real_t(i) / freq;
        const auto rad = solver.forward(t_val);
        curve.emplace_back(from.rotated(inv ? -rad : rad));
    }
}



void Sequencer::linear(Curve & curve, const Ray2_t<real_t> & from, const Vector2 & end_pos){
    const auto from_pos = from.org;
    const auto vec = (end_pos - from_pos);
    const auto norm = vec.normalized();
    
    const TrapezoidSolver solver{
        limits_.max_acc, 
        limits_.max_spd, 
        vec.length()
    };

    const auto freq = paras_.freq;
    const auto n = size_t(solver.period() * freq);
    
    curve.reserve(curve.size() + n);
    
    for(size_t i = 0; i < n; i++){
        const auto t_val = real_t(i) / freq;
        curve.emplace_back(Ray2_t<real_t>(from.org + norm * solver.forward(t_val), from.rad));
    }
}



void Sequencer::arc(Curve & curve, const Ray2_t<real_t> & from, const Ray2_t<real_t> & to, const real_t & radius){
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

void Sequencer::sideways(Curve & curve, const Ray2_t<real_t> & from, const Ray2_t<real_t> & to){
    auto from_norm_line = from.normal();
    auto to_norm_line = to.normal();

    if(from_norm_line.parallel_with(to_norm_line)){
        this->linear(curve, from, to.org);
        return;
    }

    auto mid_p_opt = from_norm_line.intersection(to_norm_line);
    if(!mid_p_opt) return;

    auto mid_p = mid_p_opt.value();
    
    this->linear(curve, Ray2_t<real_t>{from.org, from.rad}, mid_p);
    this->rotate(curve, Ray2_t<real_t>{mid_p, from.rad}, to.rad);
    this->linear(curve, Ray2_t<real_t>{mid_p, to.rad}, to.org);
}

void Sequencer::follow(Curve & curve, const Ray2_t<real_t> & from, const Ray2_t<real_t> & to){
    const auto from_line = Line2_t<real_t>(from);
    const auto to_line = Line2_t<real_t>(to);

    if(from_line.parallel_with(to_line)){
        this->linear(curve, from, to.org);
        return;
    }

    auto mid_p_opt = from_line.intersection(to_line);
    if(!mid_p_opt) return;

    auto mid_p = mid_p_opt.value();
    
    this->rotate(curve, from, from_line.rad);
    this->linear(curve, Ray2_t<real_t>{from.org, from_line.rad}, mid_p);
    this->rotate(curve, Ray2_t<real_t>{mid_p, from_line.rad}, to_line.rad);
    this->linear(curve, Ray2_t<real_t>{mid_p, to_line.rad}, to.org);
    this->rotate(curve, Ray2_t<real_t>{to.org, to_line.rad}, to.rad);
}

void Sequencer::shift(Curve & curve, const Ray2_t<real_t> & from, const Ray2_t<real_t> & to){
    this->linear(curve, from, to.org);
}


void Sequencer::spin(Curve & curve, const Ray2_t<real_t> & from, const Ray2_t<real_t> & to){
    this->rotate(curve, from, to.rad);
}

void Sequencer::wait(Curve & curve, const Ray2_t<real_t> & from, const real_t & dur){
    const auto freq = paras_.freq;
    const auto n = size_t(dur * freq);
    // const auto n = 22;
    
    curve.reserve(curve.size() + n);
    for(size_t i = 0; i < n; i++){
        curve.emplace_back(from);
    }
}