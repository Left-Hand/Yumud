#pragma once

#include "types/matrix/matrix.hpp"

template <typename T>
inline void AngleAxisRotatePoint(
    std::span<const T,3> angle_axis,
    std::span<const T,3> pt,
    std::span<T,3> result
){
    const T theta2 = square(angle_axis[0]) + square(angle_axis[1]) + square(angle_axis[2]);
    if (theta2 > T(std::numeric_limits<T>::epsilon()))
    {
        // Away from zero, use the rodriguez formula
        //
        //   result = pt costheta +
        //            (w x pt) * sintheta +
        //            w (w . pt) (1 - costheta)
        //
        // We want to be careful to only evaluate the square root if the
        // norm of the angle_axis vector is greater than zero. Otherwise
        // we get a division by zero.
        //
        const T theta = sqrt(theta2);
        const T costheta = cos(theta);
        const T sintheta = sin(theta);
        const T theta_inverse = T(1.0) / theta;

        const T w[3] = {angle_axis[0] * theta_inverse,
                        angle_axis[1] * theta_inverse,
                        angle_axis[2] * theta_inverse};

        // Explicitly inlined evaluation of the cross product for
        // performance reasons.
        const T w_cross_pt[3] = {w[1] * pt[2] - w[2] * pt[1],
                                w[2] * pt[0] - w[0] * pt[2],
                                w[0] * pt[1] - w[1] * pt[0]};
        const T tmp =
            (w[0] * pt[0] + w[1] * pt[1] + w[2] * pt[2]) * (T(1.0) - costheta);

        result[0] = pt[0] * costheta + w_cross_pt[0] * sintheta + w[0] * tmp;
        result[1] = pt[1] * costheta + w_cross_pt[1] * sintheta + w[1] * tmp;
        result[2] = pt[2] * costheta + w_cross_pt[2] * sintheta + w[2] * tmp;
    }
    else
    {
        // Near zero, the first order Taylor approximation of the rotation
        // matrix R corresponding to a vector w and angle w is
        //
        //   R = I + hat(w) * sin(theta)
        //
        // But sintheta ~ theta and theta * w = angle_axis, which gives us
        //
        //  R = I + hat(w)
        //
        // and actually performing multiplication with the point pt, gives us
        // R * pt = pt + w x pt.
        //
        // Switching to the Taylor expansion near zero provides meaningful
        // derivatives when evaluated using Jets.
        //
        // Explicitly inlined evaluation of the cross product for
        // performance reasons.
        const T w_cross_pt[3] = {angle_axis[1] * pt[2] - angle_axis[2] * pt[1],
                                angle_axis[2] * pt[0] - angle_axis[0] * pt[2],
                                angle_axis[0] * pt[1] - angle_axis[1] * pt[0]};

        result[0] = pt[0] + w_cross_pt[0];
        result[1] = pt[1] + w_cross_pt[1];
        result[2] = pt[2] + w_cross_pt[2];
    }
}

