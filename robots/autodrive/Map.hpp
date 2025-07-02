#pragma once

#include "sequence/SequenceUtils.hpp"

//描述地图所有元素的相关类与方法
namespace ymd::robots{

//描述地图元素分布的位置和朝向
struct Map{
    static constexpr Ray2<real_t> garbage_gest =              
        {Vector2{real_t(0),         real_t(0)},          real_t(PI/2)};
    static constexpr Ray2<real_t> entry_gest =                
        {Vector2{real_t(0.3),       real_t(0.3)},        real_t(PI/2)};
    static constexpr Ray2<real_t> exit_gest =                 
        {Vector2{real_t(0.3),       real_t(0.3)},        real_t(PI)};
    static constexpr Ray2<real_t> billboard_gest =            
        {Vector2{real_t(0.065),     real_t(0.3)},        real_t(-PI/2)};
    static constexpr Ray2<real_t> staging_gest =              
        {Vector2{real_t(1.8),       real_t(1.015)},      real_t(0)};
    static constexpr Ray2<real_t> raw_material_gest =         
        {Vector2{real_t(1.45),      real_t(0.3)},        real_t(-PI/2)};
    static constexpr Ray2<real_t> rough_process_gest =        
        {Vector2{real_t(1.05),      real_t(1.72)},       real_t(PI/2)};

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

    Vector2<real_t> to_pos(const Map & map) const;
    real_t to_rot(const Map & map) const;
    Ray2<real_t> to_ray(const Map & map) const;
};


}