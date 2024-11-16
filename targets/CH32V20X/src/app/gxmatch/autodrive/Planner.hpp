#pragma once 

#include "Map.hpp"
#include "sequence/Sequencer.hpp"
#include "sequence/SequenceUtils.hpp"
#include <variant>

namespace gxm{

struct Path2D{
    // std::variant<>
};

//根据具体的地图 从指定的目标设计合适的几何路线
class Planner{
protected:
    const Map & map_;
    Sequencer & sequencer_;
    Curve curve_;
public:
    Planner(const Map & map, Sequencer & sequencer):
        map_(map), sequencer_(sequencer){}

    void plan(const Ray & face, const Field & from, const Field & to);

    const auto & curve() const {
        return curve_;
    }
};


}