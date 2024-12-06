#include "Planner.hpp"
#include "../machine/chassis_actions.hpp"

using namespace gxm;

using namespace gxm::ChassisActions;

void Planner::init(){
    presets_.add(PlanPreset{FieldType::Garbage, FieldType::Staging, [this](){
        inst_.shift(map_.entry_gest.org - map_.garbage_gest.org);
        // inst_.wait(config_.duration);
        inst_.follow(map_.staging_gest - map_.entry_gest);
        // inst_.rota(map_.staging_gest.org - map_.entry_gest.org);
    }});
    
    presets_.add(PlanPreset{FieldType::Staging, FieldType::RoughProcess, [this](){
        inst_.spin(Field(FieldType::RoughProcess).to_rot(map_) - Field(FieldType::Staging).to_rot(map_));
        inst_.sideways((Field(FieldType::RoughProcess).to_pos(map_) - Field(FieldType::Staging).to_pos(map_)).length());
    }});

    presets_.add(PlanPreset{FieldType::RoughProcess, FieldType::Garbage, [this](){
        inst_.sideways((map_.exit_gest.org - map_.rough_process_gest.org).length());
        // inst_.wait(config_.duration);
        inst_.shift(map_.garbage_gest.org - map_.exit_gest.org);
    }});



    presets_.add(PlanPreset{FieldType::Garbage, FieldType::RoughProcess, [this](){
        inst_ << new ShiftAction(inst_, {0.255_r, 0.155_r});
        inst_ << new StraightAction(inst_, 1.74_r);
        inst_ << new StrictSpinAction(inst_, real_t(-PI/2));
        inst_ << new StraightAction(inst_, 0.805_r);
        inst_ << new StrictSpinAction(inst_, real_t(PI/2));
    }});
    
    presets_.add(PlanPreset{FieldType::RoughProcess, FieldType::Staging, [this](){
        inst_ << new StrictSpinAction(inst_, real_t(-PI/2));
        inst_ << new StraightAction(inst_, 0.845_r);
        inst_ << new StrictSpinAction(inst_, real_t(-PI/2));
        inst_ << new StraightAction(inst_, 0.850_r);
        inst_ << new StrictSpinAction(inst_, real_t(PI/2));
    }});

    presets_.add(PlanPreset{FieldType::Staging, FieldType::Garbage, [this](){
        inst_ << new StrictSpinAction(inst_, real_t(-PI/2));
        inst_ << new StraightAction(inst_, 0.850_r);
        inst_ << new StrictSpinAction(inst_, real_t(-PI/2));
        inst_ << new StraightAction(inst_, 1.65_r);
        inst_ << new ShiftAction(inst_, {-0.185_r, 0.265_r});
    }});

}

void Planner::wait(const real_t time){
    inst_.wait(time);
}

void Planner::plan(const Field & from, const Field & to){
    // const auto face = 
    if(presets_.trysolve(from, to)){
    // if(false){

    }else{
        inst_.follow(to.to_ray(map_) - from.to_ray(map_));
    }
}
