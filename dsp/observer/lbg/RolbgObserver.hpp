#include "core/math/real.hpp"

namespace ymd::foc {

class RolbgObserver{
public:
    // RolbgObserver(const iq_t<16> _h, const iq_t<16> _Ls, const iq_t<16> _Rs, const iq_t<16> _Tc, const iq_t<16> _omega);
    RolbgObserver(){;}

    void init();

    void reset();
    
    void update(iq_t<16> Valpha, iq_t<16> Vbeta, iq_t<16> Ialpha, iq_t<16> Ibeta);
    iq_t<16> theta() const {
        return _theta;
    }

public:

    iq_t<16> _i_alpha;
    iq_t<16> _i_beta;

    iq_t<16> _e_alpha;
    iq_t<16> _e_beta;

    iq_t<16> _theta;
};


class LesObserver{
public:
    LesObserver(const uint fc, const iq_t<16> b0, const iq_t<16> wc):
        fc_(fc), b0_(b0), wo_(3 * wc){;}

    void update(const iq_t<16> w_meas, const iq_t<16> qcurr_ref){
        const iq_t<16> err = w_hat_ - w_meas;

        const iq_t<16> last_f_ = f_;

        w_hat_ += last_f_ - 2 * wo_ * err + b0_ * qcurr_ref / fc_;
        f_ += - (wo_ * wo_) * err / fc_;
    }
private:
    uint fc_;
    iq_t<16> b0_;
    iq_t<16> wo_;

    iq_t<16> w_hat_ = 0;
    iq_t<16> f_ = 0;
};


class SpeedLoopCtrl{
public:
    SpeedLoopCtrl(const uint fc, const iq_t<16> b0, const iq_t<16> wc):
        fc_(fc), b0_(b0), wc_(wc), wo_(3 * wc){;}

    void update(const iq_t<16> w_meas, const iq_t<16> qcurr_ref){
        const iq_t<16> err = w_meas - w_hat_;

        const iq_t<16> last_f_hat_ = f_hat_;

        w_hat_ += last_f_hat_ + 2 * wo_ * err + b0_ * qcurr_ref / fc_;
        f_hat_ += (wo_ * wo_) * err / fc_;

        q_curr_ = (wc_ * err - f_hat_) / b0_ / fc_;
    }

    iq_t<16> output() const {return q_curr_;}
private:
    uint fc_;
    iq_t<16> b0_;
    iq_t<16> wc_;
    iq_t<16> wo_;

    iq_t<16> w_hat_ = 0;
    iq_t<16> f_hat_ = 0;

    iq_t<16> q_curr_;
};

}