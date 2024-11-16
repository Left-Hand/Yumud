#include "Planner.hpp"

using namespace gxm;

void Planner::init(){
    presets_.add({FieldType::Garbage, FieldType::Staging, [this](Curve & curve, const Ray & face){
        this->sequencer_.shift(curve, face, map_.entry_gest);
        this->sequencer_.follow(curve, map_.entry_gest, map_.staging_gest);
        // this->sequencer_.sideways(curve, map_.entry_gest, map_.staging_gest);
    }});
    
    presets_.add({FieldType::Staging, FieldType::RoughProcess, [this](Curve & curve, const Ray & face){
        this->sequencer_.sideways(curve, face, Field(FieldType::RoughProcess).to_ray(map_));
    }});

    presets_.add({FieldType::RoughProcess, FieldType::Garbage, [this](Curve & curve, const Ray & face){
        this->sequencer_.sideways(curve, face, map_.exit_gest);
        this->sequencer_.shift(curve, map_.exit_gest, map_.garbage_gest);
    }});
}
void Planner::plan(Curve & curve, const Ray & face, const Field & from, const Field & to){
    if(presets_.trysolve(curve, face, from, to)){
        //pass
    }else{
        sequencer_.follow(curve, face, to.to_ray(map_));
    }
}
