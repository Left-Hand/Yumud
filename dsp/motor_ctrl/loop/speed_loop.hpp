// class SpeedLoopCtrl{
// public:
//     SpeedLoopCtrl(const uint fc, const q16 b0, const q16 wc):
//         fc_(fc), 
//         b0_(b0), 
//         wc_(wc), 
//         wo_(3 * wc){;}

//     void update(const q16 w_meas, const q16 qcurr_ref){
//         const q16 err = w_meas - w_hat_;

//         const q16 last_f_hat_ = f_hat_;

//         w_hat_ += last_f_hat_ + 2 * wo_ * err + b0_ * qcurr_ref / fc_;
//         f_hat_ += (wo_ * wo_) * err / fc_;

//         q_curr_ = (wc_ * err - f_hat_) / b0_ / fc_;
//     }

//     q16 output() const {return q_curr_;}
// private:
//     uint fc_ = 0;
//     q16 b0_ = 0;
//     q16 wc_ = 0;
//     q16 wo_ = 0;

//     q16 w_hat_ = 0;
//     q16 f_hat_ = 0;

//     q16 q_curr_ = 0;
// };