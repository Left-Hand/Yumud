#include "iqt.hpp"

__fast_inline iq_t sign(const iq_t & iq){
    if(iq.value){
        iq_t ret = iq_t();
        ret.value |= (iq.value & 0x80000000);
        return ret;
    }else return iq_t(0);
}

bool is_equal_approx(const iq_t & a, const iq_t & b) {
    // Check for exact equality first, required to handle "infinity" values.
    if (a == b) {
        return true;
    }
    // Then check for approximate equality.
    iq_t tolerance = CMP_EPSILON * abs(a);
    if (tolerance < CMP_EPSILON) {
        tolerance = CMP_EPSILON;
    }
    return abs(a - b) < tolerance;
}

bool is_equal_approx_ratio(const iq_t a, const iq_t & b, iq_t epsilon, iq_t min_epsilon){

    iq_t diff = abs(a - b);
    if (diff == 0.0 || diff < min_epsilon) {
        return true;
    }
    iq_t avg_size = (abs(a) + abs(b)) / 2.0;
    diff /= avg_size;
    return diff < epsilon;
}

