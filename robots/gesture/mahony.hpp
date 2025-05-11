#pragma once

#include "core/math/real.hpp"
#include "types/quat/quat.hpp"


#include "core/math/fast/conv.hpp"

namespace ymd{
class Mahony{
public:
    using Quat = Quat_t<q24>;
    using Vector3 = Vector3_t<q24>;
protected:
    q24 dt_;

    q24 ki_;
    q24 kp_;
    uint fs_;
    Vector3 gyr_hat_;
    Vector3 inte_;
	Quat q;
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
        inte_ = Vector3();
    }

    void update(const Vector3 & gyr,const Vector3 & acc);

    void myupdate(const Vector3 & gyr,const Vector3 & acc);

    void update_v2(const Vector3 & gyr,const Vector3 & acc);
    
    // void update(const Vector3 & gyr,const Vector3 & acc, const Vector3 & mag);

    Quat result() const {return q;}
};

template<typename T>
__fast_inline OutputStream & operator<<(OutputStream & os, const Norm_t<T> & value){
    return os << T(value);
}

}