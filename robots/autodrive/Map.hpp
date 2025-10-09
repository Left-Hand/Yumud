#pragma once

#include "sequence/SequenceUtils.hpp"

//描述地图所有元素的相关类与方法
namespace ymd::robots{

//描述地图元素分布的位置和朝向
struct Map{
    static constexpr Ray2<real_t> garbage_gest =              
        {Vec2{real_t(0),         real_t(0)},          90_deg};
    static constexpr Ray2<real_t> entry_gest =                
        {Vec2{real_t(0.3),       real_t(0.3)},        90_deg};
    static constexpr Ray2<real_t> exit_gest =                 
        {Vec2{real_t(0.3),       real_t(0.3)},        180_deg};
    static constexpr Ray2<real_t> billboard_gest =            
        {Vec2{real_t(0.065),     real_t(0.3)},        -90_deg};
    static constexpr Ray2<real_t> staging_gest =              
        {Vec2{real_t(1.8),       real_t(1.015)},      0_deg};
    static constexpr Ray2<real_t> raw_material_gest =         
        {Vec2{real_t(1.45),      real_t(0.3)},        -90_deg};
    static constexpr Ray2<real_t> rough_process_gest =        
        {Vec2{real_t(1.05),      real_t(1.72)},       90_deg};

    Map & operator =(const Map & other) = default;
    Map & operator =(Map && other) = default;
};

////描述地图元素的类
class Field{
public:
    
    ////描述地图元素类型的枚举
    enum class FieldKind:uint8_t{
        Any,
        Garbage,
        Billboard,
        Staging,
        RawMaterial,
        RoughProcess
    };


    FieldKind kind_;
public:
    constexpr Field(FieldKind kind):kind_(kind){}
    constexpr FieldKind kind() const{return kind_;}
    constexpr Field & operator = (const FieldKind kind){kind_ = kind; return *this;}
    constexpr bool operator ==(const FieldKind kind) const {return kind_ == kind;}
    constexpr bool operator ==(const Field other) const {return kind_ == other.kind_;}

    [[nodiscard]] Vec2<real_t> to_position(const Map & map) const {
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

    [[nodiscard]] Angle<real_t> to_angle(const Map & map) const {
        switch(kind_){
            default:
                HALT;
            case FieldKind::Billboard:
                return (map.billboard_gest).orientation;
            case FieldKind::Garbage:
                return (map.garbage_gest).orientation;
            case FieldKind::RawMaterial:
                return (map.raw_material_gest).orientation;
            case FieldKind::RoughProcess:
                return (map.rough_process_gest).orientation;
            case FieldKind::Staging:
                return (map.staging_gest).orientation;
        }
    }

    [[nodiscard]] Ray2<real_t> to_isometry(const Map & map) const{
        return {to_pos(map), to_rot(map)};
    }
};


}