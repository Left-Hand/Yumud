#include "Sequencer.hpp"
#include "TrapezoidSolver_t.hpp"
#include "../geometry/geometryUtils.hpp"

using namespace ymd;
using namespace ymd::geometry;
using namespace ymd::robots;


void Sequencer::rotate(Curve & curve, const Ray2<iq16> & from, const Angle<iq16> end_angle){

    const TrapezoidSolver<iq16> solver{
        {limits_.max_agr, 
        limits_.max_gyr}, 
        (end_angle - from.orientation).abs().to_radian() 
    };

    const bool inv = end_angle < from.orientation;
    const auto freq = paras_.freq;
    const auto n = size_t(solver.elapsed() * freq);
    
    curve.reserve(curve.size() + n);
    
    for(size_t i = 0; i < n; i++){
        const auto t_val = iq16(i) / freq;
        const auto orientation = Angle<iq16>::from_radian(solver.forward(t_val));
        curve.emplace_back(from.rotated(inv ? -orientation : orientation));
    }
}



void Sequencer::linear(Curve & curve, const Ray2<iq16> & from, const Vec2<iq16> & end_pos){
    const auto from_pos = from.org;
    const auto vec = (end_pos - from_pos);
    const auto norm = vec.normalized();
    
    const TrapezoidSolver<iq16> solver{
        {limits_.max_acc, 
        limits_.max_spd}, 
        vec.length()
    };

    const auto freq = paras_.freq;
    const auto n = size_t(solver.elapsed() * freq);
    
    curve.reserve(curve.size() + n);
    
    for(size_t i = 0; i < n; i++){
        const auto t_val = iq16(i) / freq;
        curve.emplace_back(Ray2<iq16>(from.org + norm * solver.forward(t_val), from.orientation));
    }
}



void Sequencer::arc(
    Curve & curve, 
    const Ray2<iq16> & from, 
    const Ray2<iq16> & to, 
    const iq16 radius
){
    const auto may_center = calculate_fillet_center(from.normal(), to.normal(), radius);
    if(may_center.is_none()) return;
    // const auto center = may_center.unwrap();
    
    // TrapezoidSolver<iq16> solver{
    //     limits_.max_agr, 
    //     limits_.max_gyr, 
    //     ABS(end_angle - from.orientation)    
    // };

    // bool inv = end_angle < from.orientation;
    // const auto freq = paras_.freq;
    // const auto n = size_t(int(solver.elapsed() * freq));
    
    // curve.reserve(curve.size() + n);
    // for(size_t i = 0; i < n; i++){
    //     const auto t_val = iq16(i) / freq;
    //     auto orientation = solver.forward(t_val);
    //     curve.push_back(from.rotated(inv ? -orientation : orientation));
    // }
}

void Sequencer::sideways(Curve & curve, const Ray2<iq16> & from, const Ray2<iq16> & to){
    auto from_norm_line = from.normal();
    auto to_norm_line = to.normal();

    if(from_norm_line.parallel_with(to_norm_line)){
        this->linear(curve, from, to.org);
        return;
    }

    auto may_mid_point = from_norm_line.intersection(to_norm_line);
    if(may_mid_point.is_none()) return;

    auto mid_p = may_mid_point.unwrap();
    
    this->linear(curve, Ray2<iq16>{from.org, from.orientation}, mid_p);
    this->rotate(curve, Ray2<iq16>{mid_p, from.orientation}, to.orientation);
    this->linear(curve, Ray2<iq16>{mid_p, to.orientation}, to.org);
}

void Sequencer::follow(Curve & curve, const Ray2<iq16> & from, const Ray2<iq16> & to){
    const auto from_line = from.to_line();
    const auto to_line = to.to_line();

    if(from_line.parallel_with(to_line)){
        this->linear(curve, from, to.org);
        return;
    }

    auto may_mid_point = from_line.intersection(to_line);
    if(may_mid_point.is_none()) return;

    auto mid_p = may_mid_point.unwrap();
    
    this->rotate(curve, from, from_line.orientation);
    this->linear(curve, Ray2<iq16>{from.org, from_line.orientation}, mid_p);
    this->rotate(curve, Ray2<iq16>{mid_p, from_line.orientation}, to_line.orientation);
    this->linear(curve, Ray2<iq16>{mid_p, to_line.orientation}, to.org);
    this->rotate(curve, Ray2<iq16>{to.org, to_line.orientation}, to.orientation);
}

void Sequencer::shift(Curve & curve, const Ray2<iq16> & from, const Ray2<iq16> & to){
    this->linear(curve, from, to.org);
}


void Sequencer::spin(Curve & curve, const Ray2<iq16> & from, const Ray2<iq16> & to){
    this->rotate(curve, from, to.orientation);
}

void Sequencer::wait(Curve & curve, const Ray2<iq16> & from, const iq16 dur){
    const auto freq = paras_.freq;
    const auto n = size_t(dur * freq);
    // const auto n = 22;
    
    curve.reserve(curve.size() + n);
    for(size_t i = 0; i < n; i++){
        curve.emplace_back(from);
    }
}