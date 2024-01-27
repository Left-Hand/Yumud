// #include "KalmanFilter.hpp"

// KalmanFilter::KalmanFilter(real_t r, real_t q, real_t a, real_t b, real_t c):
//     R(r), Q(q), A(a), B(b), C(c){;}


// const real_t KalmanFilter::update(const real_t & z) {
//     if (!init) {
//         x = real_t(1.0f) / C * z;
//         cov = Q / (C * C);
//         init = true;
//     }

//     else {
//         real_t pred = predict();
//         real_t p_cov = uncertainty();

//         real_t K = p_cov * C / (C * C * p_cov + Q);

//         x = pred + K * (z - C * pred);
//         cov = p_cov - K * C * p_cov;
//     }

//   return real_t(x);
// }

// const real_t KalmanFilter::predict() {
//   return A * x;
// }

// const real_t KalmanFilter::uncertainty() {
//   return A * A * cov + R;
// }
