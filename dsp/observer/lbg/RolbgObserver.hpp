#include "core/math/real.hpp"

namespace ymd::foc {

class RolbgObserver{
public:
    // RolbgObserver(const q16 _h, const q16 _Ls, const q16 _Rs, const q16 _Tc, const q16 _omega);
    RolbgObserver(){
        reset();
    }

    void reset();
    
    void update(q16 Valpha, q16 Vbeta, q16 Ialpha, q16 Ibeta);
    q16 theta() const {
        return theta_;
    }

public:

    q16 i_alpha_ = {};
    q16 i_beta_ = {};

    q16 e_alpha_ = {};
    q16 e_beta_ = {};

    q16 theta_ = {};
};


class LesObserver{
public:
    LesObserver(const uint fc, const q16 b0, const q16 wc):
        fc_(fc), b0_(b0), wo_(3 * wc){;}

    void update(const q16 w_meas, const q16 qcurr_ref){
        const q16 err = w_hat_ - w_meas;

        const q16 last_f_ = f_;

        w_hat_ += last_f_ - 2 * wo_ * err + b0_ * qcurr_ref / fc_;
        f_ += - (wo_ * wo_) * err / fc_;
    }
private:
    uint fc_;
    q16 b0_;
    q16 wo_;

    q16 w_hat_ = 0;
    q16 f_ = 0;
};


class SpeedLoopCtrl{
public:
    SpeedLoopCtrl(const uint fc, const q16 b0, const q16 wc):
        fc_(fc), b0_(b0), wc_(wc), wo_(3 * wc){;}

    void update(const q16 w_meas, const q16 qcurr_ref){
        const q16 err = w_meas - w_hat_;

        const q16 last_f_hat_ = f_hat_;

        w_hat_ += last_f_hat_ + 2 * wo_ * err + b0_ * qcurr_ref / fc_;
        f_hat_ += (wo_ * wo_) * err / fc_;

        q_curr_ = (wc_ * err - f_hat_) / b0_ / fc_;
    }

    q16 output() const {return q_curr_;}
private:
    uint fc_;
    q16 b0_;
    q16 wc_;
    q16 wo_;

    q16 w_hat_ = 0;
    q16 f_hat_ = 0;

    q16 q_curr_;
};

}