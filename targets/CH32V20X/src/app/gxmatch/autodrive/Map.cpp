#include "Map.hpp"


using namespace gxm;

Vector2 Area::to_pos(const Map & map) const {
    switch(type_){
        default:
            HALT;
        case AreaType::Billboard:
            return map.billboard_gest.org;
        case AreaType::Garbage:
            return map.garbage_gest.org;
        case AreaType::RawMaterial:
            return map.raw_material_gest.org;
        case AreaType::RoughProcess:
            return map.rough_process_gest.org;
        case AreaType::Staging:
            return map.staging_gest.org;
    }
}

real_t Area::to_rot(const Map & map) const {
    switch(type_){
        default:
            HALT;
        case AreaType::Billboard:
            return map.billboard_gest.rad;
        case AreaType::Garbage:
            return map.garbage_gest.rad;
        case AreaType::RawMaterial:
            return map.raw_material_gest.rad;
        case AreaType::RoughProcess:
            return map.rough_process_gest.rad;
        case AreaType::Staging:
            return map.staging_gest.rad;
    }
}

