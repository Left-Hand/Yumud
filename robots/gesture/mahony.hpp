#pragma once

#include "core/math/real.hpp"
#include "types/vectors/quat.hpp"


#include "core/math/fast/conv.hpp"

namespace ymd{
class Mahony{
public:
    using IQuat = Quat<q24>;
    using IV3 = Vec3<q24>;

public:
    struct Config{
        q24 kp;
        q24 ki;
        uint fs;
    };

    Mahony(const Config & cfg){
        reconf(cfg);
    }


    void reconf(const Config & cfg){
        ki_ = cfg.ki;
        kp_ = cfg.kp;    
        dt_ = q24(1) / cfg.fs;
        fs_ = cfg.fs;
    }

    void reset(){
        inte_ = IV3::ZERO;
    }

    void update(const IV3 & gyr,const IV3 & acc);

    void myupdate(const IV3 & gyr,const IV3 & acc);

    void update_v2(const IV3 & gyr,const IV3 & acc);
    void myupdate_v2(const IV3 & gyr,const IV3 & acc);
    
    // void update(const IV3 & gyr,const IV3 & acc, const IV3 & mag);

    IQuat result() const {return q;}

private:
protected:
    q24 dt_ = 0;

    q24 ki_ = 0;
    q24 kp_ = 0;
    q24 kd_ = 0;
    uint fs_ = 0;
    IV3 gyr_hat_ = IV3::ZERO;
    IV3 inte_ = IV3::ZERO;
	IQuat q = IQuat::IDENTITY;
};

template<typename T>
__fast_inline OutputStream & operator<<(OutputStream & os, const Norm<T> & value){
    return os << T(value);
}

}