#include "Map.hpp"


using namespace gxm;

Vector2 Field::to_pos(const Map & map) const {
    switch(type_){
        default:
            HALT;
        case FieldType::Billboard:
            return Ray(map.billboard_gest).org;
        case FieldType::Garbage:
            return Ray(map.garbage_gest).org;
        case FieldType::RawMaterial:
            return Ray(map.raw_material_gest).org;
        case FieldType::RoughProcess:
            return Ray(map.rough_process_gest).org;
        case FieldType::Staging:
            return Ray(map.staging_gest).org;
    }
}

real_t Field::to_rot(const Map & map) const {
    switch(type_){
        default:
            HALT;
        case FieldType::Billboard:
            return Ray(map.billboard_gest).rad;
        case FieldType::Garbage:
            return Ray(map.garbage_gest).rad;
        case FieldType::RawMaterial:
            return Ray(map.raw_material_gest).rad;
        case FieldType::RoughProcess:
            return Ray(map.rough_process_gest).rad;
        case FieldType::Staging:
            return Ray(map.staging_gest).rad;
    }
}

Ray Field::to_ray(const Map & map) const{
    return {to_pos(map), to_rot(map)};
}