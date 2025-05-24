#pragma once

#include "core/platform.hpp"
#include "types/vector2/vector2.hpp"
#include "robots/foc/remote/remote.hpp"
#include "machine_concepts.hpp"

class Joint{


};

class Axis{
public:
    RemoteFOCMotor & motor;
private:
    real_t cache = real_t(-114.514);
    const real_t ratio;
    const Range targ_mm_limit;
public:
    Axis(RemoteFOCMotor & _motor, const real_t _ratio, const Range _targ_mm_limit):motor(_motor), ratio(_ratio), targ_mm_limit(_targ_mm_limit){}
    void setTargetMM(const real_t mm){
        const real_t res = targ_mm_limit.clamp(mm) * ratio;
        if(res != cache){
            cache = res;
            motor.setTargetPosition(res);
        }
    }

    real_t readMM() const {
        return motor.readPosition() / ratio;
    }

    bool hasReachedMM(const real_t mm) const{
        return motor.reached(mm * ratio);
    }
};

struct Nozzle{

};

class MachineConcept{
public:
    virtual bool reached(const real_t x) const = 0;
};

class X_Machine:public MachineConcept{
public:
    Axis x_axis;
protected:
    scexpr real_t x_scale = real_t(1.0)/40;

public:
    X_Machine(const Axis & _x_axis):x_axis(_x_axis){;}
    void x_mm(const real_t x){
        x_axis.setTargetMM(x);
    }

    bool reached(const real_t x) const override {
        return x_axis.hasReachedMM(x);
    }
};



class Y_Machine:public MachineConcept{
public:
    Axis y_axis;
protected:
    scexpr real_t y_scale = real_t(1.0)/40;

public:
    Y_Machine(const Axis & _y_axis):y_axis(_y_axis){;}
    void y_mm(const real_t x){
        y_axis.setTargetMM(x);
    }

    bool reached(const real_t x) const override{
        return y_axis.hasReachedMM(x);
    }
};



struct XY_Machine:public X_Machine, public Y_Machine{
private:
    using Vector2 = Vector2<real_t>;
    bool reached(const real_t x) const override{
        return false;
    }
public:
    XY_Machine(const Axis & _x_axis, const Axis & _y_axis):
            X_Machine(_x_axis), Y_Machine(_y_axis){;}
    void xy_mm(const real_t x, const real_t y){
        x_mm(x);
        y_mm(y);
    }

    void xy_mm(const Vector2 & v){
        xy_mm(v.x, v.y);
    }

    Vector2 xy_mm(){
        return Vector2(x_axis.readMM(), y_axis.readMM());
    }

    bool reached(const Vector2 & position) const {
        return X_Machine::reached(position.x) and Y_Machine::reached(position.x);
    }
};

class Z_Machine:public MachineConcept{
public:
    Axis z_axis;
protected:
    real_t z_cache = 0;
public:
    Z_Machine(const Axis & _z_axis):z_axis(_z_axis){;}
    void z_mm(const real_t z){
        z_axis.setTargetMM(z);
        z_cache = z;
    }

    bool reached(const real_t x) const override{
        return z_axis.hasReachedMM(x);
    }

    bool reached() const {
        return z_axis.hasReachedMM(z_cache);
    }
};

class Nozzle_Machine:public Z_Machine{
protected:
    scexpr real_t z_scale = real_t(1.0)/2;

public:
    Nozzle_Machine(const Axis & _z_axis):Z_Machine(_z_axis){;}
    virtual void z_pick() = 0;
    virtual void z_hold() = 0;
    virtual void z_place() = 0;
    virtual void z_idle() = 0;
    virtual void z_inspect() = 0;
    virtual void z_release() = 0;
};

struct CoreXY{

};

struct Cantilever{

};