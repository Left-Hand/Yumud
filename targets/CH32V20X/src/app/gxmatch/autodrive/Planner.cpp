#include "Planner.hpp"

using namespace gxm;

void Planner::plan(const Ray & face, const Field & from, const Field & to){
    // auto curr_pos = face.org;
    // auto from_pos = from.to_pos(map_);
    // auto to_pos = to.to_pos(map_);

    // switch()
    sequencer_.fillet(curve_, face, to.to_gest(map_));
}
