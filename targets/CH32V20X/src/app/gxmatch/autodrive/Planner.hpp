#pragma once 

#include "Map.hpp"
#include "sequence/Sequencer.hpp"
#include "sequence/SequenceUtils.hpp"
#include <variant>
#include <functional>

namespace gxm{

struct Path2D{
    // std::variant<>
};

//根据具体的地图 从指定的目标设计合适的几何路线
class Planner{
public:
    struct PlanPreset{
        using Callback = std::function<void(Curve &, const Ray &)>;
        const FieldType from;
        const FieldType to;
        Callback cb;
    };

    class PlanPresets{
    protected:
        std::vector<PlanPreset> presets_ = {};
    public:
        void add(PlanPreset && preset){presets_.emplace_back(std::move(preset));}
        const PlanPreset * find(const Field & from, const Field & to){
            for(const auto & preset : presets_){
                if(preset.from == from and preset.to == to){
                    return &preset;
                }
            }
            return nullptr;
        }

        bool trysolve(Curve & curve, const Ray & face, const Field & from, const Field & to){
            if(auto preset = find(from, to)){
                preset->cb(curve, face);
                return true;
            }
            return false;
        }
    };
protected:
    const Map & map_;
    Sequencer & sequencer_;

    PlanPresets presets_;

    void init();
public:
    DELETE_COPY_AND_MOVE(Planner)

    Planner(const Map & map, Sequencer & sequencer):
        map_(map), sequencer_(sequencer){init();}

    void plan(Curve & curve, const Ray & face, const Field & from, const Field & to);
    void plan(Curve & curve, const Field & from, const Field & to){
        plan(curve, from.to_ray(map_), from, to);
    }

    // const auto & curve() const {
    //     return curve_;
    // }
};


}