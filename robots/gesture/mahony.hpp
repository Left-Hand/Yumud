#pragma once

#include "core/math/real.hpp"
#include "types/vectors/quat.hpp"


#include "core/math/fast/conv.hpp"

namespace ymd{
class Mahony{
public:
    using IQuat = Quat<q24>;
    using V3 = Vec3<q24>;
protected:
    q24 dt_;

    q24 ki_;
    q24 kp_;
    q24 kd_;
    uint fs_;
    V3 gyr_hat_;
    V3 inte_;
	IQuat q;
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
        inte_ = V3();
    }

    void update(const V3 & gyr,const V3 & acc);

    void myupdate(const V3 & gyr,const V3 & acc);

    void update_v2(const V3 & gyr,const V3 & acc);
    void myupdate_v2(const V3 & gyr,const V3 & acc);
    
    // void update(const V3 & gyr,const V3 & acc, const V3 & mag);

    IQuat result() const {return q;}
};

template<typename T>
__fast_inline OutputStream & operator<<(OutputStream & os, const Norm_t<T> & value){
    return os << T(value);
}

}