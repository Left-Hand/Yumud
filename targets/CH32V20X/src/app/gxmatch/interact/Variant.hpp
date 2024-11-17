#pragma once

namespace gxm{

class Variant{
public:
    enum class VariantType{
        INT,
        IQ,
        FLOAT,
        DOUBLE,
        STRING,
        CHAR,
        VECTOR2,
        VECTOR3,
        QUAT,
    };
protected:
    Variant(const Variant & other) = default;
    Variant(Variant && other) = default;
public:
    Variant copy() const{
        return *this;
    }
};

}