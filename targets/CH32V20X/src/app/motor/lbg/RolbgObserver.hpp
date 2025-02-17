#include "sys/math/real.hpp"
#include "../SensorlessObserverIntf.hpp"

namespace ymd::foc {

class RolbgObserver:public SensorlessObserverIntf{
public:
    // RolbgObserver(const iq_t _h, const iq_t _Ls, const iq_t _Rs, const iq_t _Tc, const iq_t _omega);
    RolbgObserver(){;}

    void init();

    void reset();
    
    void update(iq_t Valpha, iq_t Vbeta, iq_t Ialpha, iq_t Ibeta);
    iq_t theta() const {
        return _theta;
    }

public:

    iq_t _i_alpha;
    iq_t _i_beta;

    iq_t _e_alpha;
    iq_t _e_beta;

    iq_t _theta;
};


class LesObserver{
public:
    LesObserver(const uint fc, const iq_t b0, const iq_t wc):
        fc_(fc), b0_(b0), wo_(3 * wc){;}

    void update(const iq_t w_meas, const iq_t qcurr_ref){
        const iq_t err = w_hat_ - w_meas;

        const iq_t last_f_ = f_;

        w_hat_ += last_f_ - 2 * wo_ * err + b0_ * qcurr_ref / fc_;
        f_ += - (wo_ * wo_) * err / fc_;
    }
private:
    uint fc_;
    iq_t b0_;
    iq_t wo_;

    iq_t w_hat_ = 0;
    iq_t f_ = 0;
};


class SpeedLoopCtrl{
public:
    SpeedLoopCtrl(const uint fc, const iq_t b0, const iq_t wc):
        fc_(fc), b0_(b0), wc_(wc), wo_(3 * wc){;}

    void update(const iq_t w_meas, const iq_t qcurr_ref){
        const iq_t err = w_meas - w_hat_;

        const iq_t last_f_hat_ = f_hat_;

        w_hat_ += last_f_hat_ + 2 * wo_ * err + b0_ * qcurr_ref / fc_;
        f_hat_ += (wo_ * wo_) * err / fc_;

        q_curr_ = (wc_ * err - f_hat_) / b0_ / fc_;
    }

    iq_t output() const {return q_curr_;}
private:
    uint fc_;
    iq_t b0_;
    iq_t wc_;
    iq_t wo_;

    iq_t w_hat_ = 0;
    iq_t f_hat_ = 0;

    iq_t q_curr_;
};

}