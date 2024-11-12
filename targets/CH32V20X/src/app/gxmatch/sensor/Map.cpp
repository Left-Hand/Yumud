#include "Map.hpp"


using namespace gxm;

Area::operator Vector2() const{
    switch(type_){
        default:
            HALT;
        case AreaType::Billboard:
            return map_.billboard_pos;
        case AreaType::Garbage:
            return map_.garbage_pos;
        case AreaType::RawMaterial:
            return map_.raw_material_pos;
        case AreaType::RoughProcess:
            return map_.rough_process_pos;
        case AreaType::Staging:
            return map_.staging_pos;
    }
}
