#pragma once

#include "core/math/real.hpp"
#include "types/quat/quat.hpp"


#include "core/math/fast/conv.hpp"

namespace ymd{
class Mahony{
public:
    using Quat = Quat_t<q14>;
    using Vector3 = Vector3_t<q14>;
protected:
    q14 inv_fs_;

    q14 ki_;
    q14 kp_;
    // q14 fs;
    Vector3 inte_;
	Quat q;
public:
    struct Config{
        q14 kp;
        q14 ki;
        uint fs;
    };

    Mahony(const Config & cfg){
        reconf(cfg);
    }


    void reconf(const Config & cfg){
        ki_ = cfg.ki;
        kp_ = cfg.kp;    
        inv_fs_ = q14(1) / cfg.fs;
    }

    void reset(){
        inte_ = Vector3();
    }

    __no_inline
    void update(const Vector3 & gyr,const Vector3 & acc);

    __no_inline
    void update_v2(const Vector3 & gyr,const Vector3 & acc);
    
    // void update(const Vector3 & gyr,const Vector3 & acc, const Vector3 & mag);

    Quat result() const {return q;}
};

template<typename T>
__fast_inline OutputStream & operator<<(OutputStream & os, const Norm_t<T> & value){
    return os << T(value);
}

}