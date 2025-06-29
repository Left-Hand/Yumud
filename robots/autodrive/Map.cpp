#include "Map.hpp"


using namespace ymd;
using namespace ymd::robots;

Vector2<real_t> Field::to_pos(const Map & map) const {
    switch(kind_){
        default:
            HALT;
        case FieldKind::Billboard:
            return (map.billboard_gest).org;
        case FieldKind::Garbage:
            return (map.garbage_gest).org;
        case FieldKind::RawMaterial:
            return (map.raw_material_gest).org;
        case FieldKind::RoughProcess:
            return (map.rough_process_gest).org;
        case FieldKind::Staging:
            return (map.staging_gest).org;
    }
}

real_t Field::to_rot(const Map & map) const {
    switch(kind_){
        default:
            HALT;
        case FieldKind::Billboard:
            return (map.billboard_gest).rad;
        case FieldKind::Garbage:
            return (map.garbage_gest).rad;
        case FieldKind::RawMaterial:
            return (map.raw_material_gest).rad;
        case FieldKind::RoughProcess:
            return (map.rough_process_gest).rad;
        case FieldKind::Staging:
            return (map.staging_gest).rad;
    }
}

Ray2<real_t> Field::to_ray(const Map & map) const{
    return {to_pos(map), to_rot(map)};
}