#pragma once

#include "sys/core/system.hpp"
#include "memory.h"

namespace gxm{

enum class VariantType{
    NIL,

    U8,
    I8,
    U16,
    I16,
    U32,
    I32,
    U64,
    I64,
    INT,

    IQ,
    FLOAT,
    DOUBLE,

    CHAR,

    POLAR,
    COMPLEX,
    VECTOR2,
    COLOR,
    RANGE,
    RECT2,
    VECTOR3,
    QUAT,
    PLANE,

    ARC2D,
    CIRCLE2D,

    SEGMENT2D,
    RAY2D,
    LINE2D,

    BINA,
    GRAY,
    RGB565,
    RGB888,
    RGBA24,
    HSV888,
    LAB888
};

class Variant{
public:

protected:
    scexpr size_t N = 32;

    VariantType type_ = VariantType::NIL;
    uint8_t data_[32];

    Variant(const Variant & other) = default;
    Variant(Variant && other) = default;

    static constexpr size_t checksize(size_t size){
        if(size >= N) HALT;
        return size;
    }

    static constexpr VariantType typecheck(const VariantType & type){
        if(type == VariantType::NIL) HALT;
        return type;
    }
public:

    #define VAR_IMPLICIT_FROM_TEMPLATE(typename)\
    Variant(const typename & val){\
        memcpy(data_, reinterpret_cast<const typename *>(&val), checksize(sizeof(typename)));\
    }\

    VAR_IMPLICIT_FROM_TEMPLATE(int)
    VAR_IMPLICIT_FROM_TEMPLATE(float)
    VAR_IMPLICIT_FROM_TEMPLATE(double)

    #undef VAR_IMPLICIT_FROM_TEMPLATE



    #define VAR_IMPLICIT_TO_TEMPLATE(typename)\
    operator typename () const{\
        return *reinterpret_cast<const typename *>(data_);\
    }\

    #define VAR_EXPLICIT_TO_TEMPLATE(typename)\
    operator typename () const{\
        return *reinterpret_cast<const typename *>(data_);\
    }\

    VAR_IMPLICIT_TO_TEMPLATE(int);
    VAR_IMPLICIT_TO_TEMPLATE(float);
    VAR_IMPLICIT_TO_TEMPLATE(double);

    #undef VAR_IMPLICIT_TO_TEMPLATE
    #undef VAR_EXPLICIT_TO_TEMPLATE


    Variant copy() const{
        return *this;
    }

    __inline constexpr VariantType type() const{
        return type_;
    }

    __inline constexpr bool is(VariantType & type) const{
        return type_ == type;
    }

    bool isIntergal() const;
    bool isFloating() const;
    bool isNumber() const;
};

}