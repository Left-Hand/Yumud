#include "iq_t.hpp"
#include "sys/debug/debug_inc.h"

bool is_equal_approx(const iq_t a, const iq_t b) {
    // Check for exact equality first, required to handle "infinity" values.
    if (a == b) {
        return true;
    }
    // Then check for approximate equality.
    iq_t tolerance = iq_t(CMP_EPSILON) * abs(a);
    if (tolerance < iq_t(CMP_EPSILON)) {
        tolerance = iq_t(CMP_EPSILON);
    }
    return abs(a - b) < tolerance;
}

bool is_equal_approx_ratio(const iq_t a, const iq_t b, iq_t epsilon, iq_t min_epsilon){

    iq_t diff = abs(a - b);
    if (diff == 0 || diff < min_epsilon) {
        return true;
    }
    iq_t avg_size = (abs(a) + abs(b)) >> 1;
    diff /= avg_size;
    return diff < epsilon;
}

