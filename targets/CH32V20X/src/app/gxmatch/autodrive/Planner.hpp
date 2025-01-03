#pragma once 

#include "Map.hpp"
#include "sequence/Sequencer.hpp"
#include "sequence/SequenceUtils.hpp"
#include "../machine/chassis_module.hpp"
#include <variant>
#include <functional>

namespace gxm{

struct Path2D{
    // std::variant<>
};

//路径规划类
//根据具体的地图 从指定的目标设计合适的几何路线
class Planner{
public:

    using Config = ChassisModule::Config;

    //预设的固定路线类
    struct PlanPreset{
        using Callback = std::function<void(void)>;
        const FieldType from;
        const FieldType to;
        Callback cb;
    };

    //预设固定路线的容器

    class PlanPresets{
    protected:
        std::vector<PlanPreset> presets_ = {};
    public:

        //添加预设
        void add(PlanPreset && preset){presets_.emplace_back(std::move(preset));}
        const PlanPreset * find(const Field & from, const Field & to){
            for(const auto & preset : presets_){
                if(preset.from == from and preset.to == to){
                    return &preset;
                }
            }
            return nullptr;
        }

        //尝试从预设求解路径 如无预设返回false
        [[nodiscard]] bool trysolve(const Field & from, const Field & to){
            if(auto preset = find(from, to)){
                EXECUTE(preset->cb);
                return true;
            }
            return false;
        }
    };
protected:
    const Config & config_;
    ChassisModule & inst_;
    const Map & map_;
    // Sequencer & sequencer_;
    PlanPresets presets_;

    void init();
public:
    DELETE_COPY_AND_MOVE(Planner)

    Planner(ChassisModule & inst, const Map & map):
                config_(inst.config()), 
                inst_(inst),
                map_(map){init();}

    // //进行规划 输入要输出的路线 当前所在位置及朝向 所在区域 前往区域
    // void plan(Curve & curve, const Ray & face, const Field & from, const Field & to);

    // //进行规划 输入要输出的路线 所在区域 前往区域
    // void plan(Curve & curve, const Field & from, const Field & to){
    //     plan(curve, from.to_ray(map_), from, to);
    // }

    //进行规划 输入要输出的路线 当前所在位置及朝向 所在区域 前往区域
    void plan(const Field & from, const Field & to);
    void wait(const uint time);

    //进行规划 输入要输出的路线 所在区域 前往区域
    // void plan(const Field & from, const Field & to){
        // plan(from.to_ray(map_), from, to);
    // }
};


}