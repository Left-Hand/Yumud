#pragma once

#include "sys/math/real.hpp"
#include "types/quat/quat.hpp"


namespace ymd{
class Mahony{
public:
    using Quat = Quat_t<real_t>;
    using Vector3 = Vector3_t<real_t>;
protected:
    real_t inv_fs_;

    real_t ki_;
    real_t kp_;
    // real_t fs;
    Vector3 inte_;
	Quat q;
public:
    struct Config{
        real_t kp;
        real_t ki;
        uint fs;
    };

    Mahony(const Config & cfg){
        reconf(cfg);
    }


    void reconf(const Config & cfg){
        ki_ = cfg.ki;
        kp_ = cfg.kp;    
        inv_fs_ = real_t(1) / cfg.fs;
    }

    void reset(){
        inte_ = Vector3();
    }


    void update(const Vector3 & gyr,const Vector3 & acc);
    void update(const Vector3 & gyr,const Vector3 & acc, const Vector3 & mag);

    Quat result() const {return q;}
};

}