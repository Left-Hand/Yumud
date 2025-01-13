#include "sys/math/real.hpp"

namespace ymd::foc {

class RolbgObserver {
public:
    // RolbgObserver(const iq_t _h, const iq_t _Ls, const iq_t _Rs, const iq_t _Tc, const iq_t _omega);
    RolbgObserver(){;}

    void init();

    void reset();
    
    void update(iq_t Valpha, iq_t Vbeta, iq_t Ialpha, iq_t Ibeta);

    iq_t getEstimatedTheta() const;

    iq_t theta() const {
        return _theta;
    }

public:
    // const iq_t h;
    // const iq_t Ls;
    // const iq_t Rs;
    // const iq_t Tc;
    // const iq_t omega;

    iq_t _i_alpha;
    iq_t _i_beta;

    // BEMF 估算值
    iq_t _e_alpha;
    iq_t _e_beta;

    iq_t _theta;
};

} // namespace ymd