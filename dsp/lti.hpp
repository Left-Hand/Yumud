#pragma once

#include "core/math/matrix/static_matrix.hpp"

namespace ymd::dsp{


template<typename T, size_t S, size_t U>
struct LinearTimeInvariantModel{
    Matrix<T, S, S> A;
    Matrix<T, S, U> B;
    Matrix<T, U, U> R;
    Matrix<T, S, S> Q;
};

template<typename T, size_t S, size_t U>
static constexpr LinearTimeInvariantModel<T, S, U> make_lti_model(
    Matrix<T, S, S> A, 
    Matrix<T, S, U> B, 
    Matrix<T, U, U> R, 
    Matrix<T, S, S> Q
){
    return LinearTimeInvariantModel<T, S, U>{A, B, R, Q};
}
}