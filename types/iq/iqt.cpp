#include "iqt.hpp"

__fast_inline bool std::is_equal_approx_ratio(iq_t a, iq_t b, iq_t epsilon, iq_t min_epsilon ) {
    iq_t diff = std::abs(a - b);
    if (diff == 0.0 || diff < min_epsilon) {
        return true;
    }
    iq_t avg_size = (std::abs(a) + std::abs(b)) / 2.0;
    diff /= avg_size;
    return diff < epsilon;
}