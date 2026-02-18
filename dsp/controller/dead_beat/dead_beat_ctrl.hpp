#pragma once

// https://zhuanlan.zhihu.com/p/333910958

#include "dsp/state_vector.hpp"


namespace ymd::dsp{

template<typename T>
class DeadBeatCtrl { 
public:
    struct Config{
        T R;
        T Ld;
        T Lq;
        T phi;
        uint32_t fs;
    };

    constexpr DeadBeatCtrl(const Config & cfg){
        reconf(cfg);
    }

    constexpr void reconf(const Config & cfg){
        R_ = cfg.R;
        L_d_ = cfg.Ld;
        L_q_ = cfg.Lq;
        L_d_mul_fs_ = L_d_ * cfg.fs;
        L_q_mul_fs_ = L_q_ * cfg.fs;
        R_by_Ldfs_ = R_ / L_d_mul_fs_;
        R_by_Lqfs_ = R_ / L_q_mul_fs_;
        inv_Ldfs_ = T(1) / L_d_mul_fs_;
        inv_Lqfs_ = T(1) / L_q_mul_fs_;
        phi_ = cfg.phi;
        delta_ = T(1) / cfg.fs;
    }

    
    constexpr void update(const State i_dq_ref, const State i_dq, const iq16 omega){
        const auto i_dq_pre = comp(i_dq, u_dq_pre_, omega);
        u_dq_pre_ = predict(i_dq_ref, i_dq_pre, omega);
        last_i_dq_ = i_dq;
    }

    constexpr const auto & get() const {return u_dq_pre_;}
private:
    T R_;
    
    T L_d_;
    T L_q_;

    T L_d_mul_fs_;
    T L_q_mul_fs_;

    T R_by_Ldfs_;
    T R_by_Lqfs_;

    T inv_Ldfs_;
    T inv_Lqfs_;

    T phi_;
    T delta_;

    State last_i_dq_;
    State u_dq_pre_;
    using State = StateVector<T, 2>;
    constexpr State predict(const State i_dq_ref, const State i_dq_meas, const iq16 omega) const {
        return {
            R_ * i_dq_ref[0] + L_d_mul_fs_ * (i_dq_ref[0] - i_dq_meas[0]) - omega * L_q_ * i_dq_meas[1],
            R_ * i_dq_ref[1] + L_q_mul_fs_ * (i_dq_ref[1] - i_dq_meas[1]) + omega * L_d_ * i_dq_meas[0] - omega * phi_;
        };
    }

    constexpr State comp(const State i_dq, const State last_u_dq, const iq16 omega) const {
        return {
            (1 - R_by_Ldfs_) * i_dq[0] + delta_ * omega * i_dq[1] + last_u_dq[0] * inv_Ldfs_;
            (1 - R_by_Lqfs_) * i_dq[1] - delta_ * omega * i_dq[0] + last_u_dq[1] * inv_Lqfs_;
        }
    }

};

// https://zhuanlan.zhihu.com/p/659205719
//TODO
}