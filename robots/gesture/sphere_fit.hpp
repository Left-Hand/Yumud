#pragma once

#include <span>
#include "algebra/vectors/vec3.hpp"

//original source of：https://github.com/stevenjiaweixie/vrgimbal
//https://github.com/stevenjiaweixie/vrgimbal/blob/master/Firmware/VRGimbal/calibrationRoutines.cpp

// https://gitee.com/namelesstech/maplepilot_infineon_cyt2bl3/blob/master/maplepilot_flycontroller_namelesstech/fc_driver/control/routines.c

namespace ymd::robots{


template<typename T>
struct LeastSquaresSphere{

    struct FitResult{
        math::Vec3<T> center;
        T radius;
    };


    struct Intermediate{
        T x_sumplain ;
        T x_sumsq;
        T x_sumcube ;
        
        T y_sumplain ;
        T y_sumsq ;
        T y_sumcube ;
        
        T z_sumplain ;
        T z_sumsq ;
        T z_sumcube ;
        
        T xy_sum ;
        T xz_sum ;
        T yz_sum ;
        
        T x2y_sum ;
        T x2z_sum ;
        T y2x_sum ;
        T y2z_sum ;
        T z2x_sum ;
        T z2y_sum ;
        
        size_t len;

        constexpr void set_zero(){
            auto & self = *this;

            self.x_sumplain  = 0;
            self.x_sumsq = 0;
            self.x_sumcube  = 0;
            
            self.y_sumplain  = 0;
            self.y_sumsq  = 0;
            self.y_sumcube  = 0;
            
            self.z_sumplain  = 0;
            self.z_sumsq  = 0;
            self.z_sumcube  = 0;
            
            self.xy_sum  = 0;
            self.xz_sum  = 0;
            self.yz_sum  = 0;
            
            self.x2y_sum  = 0;
            self.x2z_sum  = 0;
            self.y2x_sum  = 0;
            self.y2z_sum  = 0;
            self.z2x_sum  = 0;
            self.z2y_sum  = 0;
            
            self.len = 0;
        }


        constexpr void push_point(
            const Vec3<T> p
        ){
            auto & self = *this;

            T x2 = p.x * p.x;
            T y2 = p.y * p.y;
            T z2 = p.z * p.z;

            self.x_sumplain += p.x;
            self.x_sumsq += x2;
            self.x_sumcube += x2 * p.x;

            self.y_sumplain += p.y;
            self.y_sumsq += y2;
            self.y_sumcube += y2 * p.y;

            self.z_sumplain += p.z;
            self.z_sumsq += z2;
            self.z_sumcube += z2 * p.z;

            self.xy_sum += p.x * p.y;
            self.xz_sum += p.x * p.z;
            self.yz_sum += p.y * p.z;

            self.x2y_sum += x2 * p.y;
            self.x2z_sum += x2 * p.z;

            self.y2x_sum += y2 * p.x;
            self.y2z_sum += y2 * p.z;

            self.z2x_sum += z2 * p.x;
            self.z2y_sum += z2 * p.y;

        }


        constexpr void push_points(
            std::span<const math::Vec3<T>> points
        ){
            auto & self = *this;
            self.len = points.size();
            for (size_t i = 0; i < self.len; i++) {
                push_point(points[i]);
            }
        }


        FitResult solve(
            size_t max_iterations, 
            T delta
        ) const {
            //
            //Least Squares Fit a sphere A,B,C with radius squared Rsq to 3D data
            //
            //    P is a structure that has been computed with the data earlier.
            //    P.npoints is the number of elements; the length of X,Y,Z are identical.
            //    P's members are logically named.
            //
            //    X[n] is the x component of point n
            //    Y[n] is the y component of point n
            //    Z[n] is the z component of point n
            //
            //    A is the x coordiante of the sphere
            //    B is the y coordiante of the sphere
            //    C is the z coordiante of the sphere
            //    Rsq is the radius squared of the sphere.
            //
            //This method should converge; maybe 5-100 iterations or more.
            //

            const T inv_len = static_cast<T>(1) / len;
            T x_sum = x_sumplain * inv_len;        //sum( X[n] )
            T x_sum2 = x_sumsq * inv_len;    //sum( X[n]^2 )
            T x_sum3 = x_sumcube * inv_len;    //sum( X[n]^3 )
            T y_sum = y_sumplain * inv_len;        //sum( Y[n] )
            T y_sum2 = y_sumsq * inv_len;    //sum( Y[n]^2 )
            T y_sum3 = y_sumcube * inv_len;    //sum( Y[n]^3 )
            T z_sum = z_sumplain * inv_len;        //sum( Z[n] )
            T z_sum2 = z_sumsq * inv_len;    //sum( Z[n]^2 )
            T z_sum3 = z_sumcube * inv_len;    //sum( Z[n]^3 )

            T XY = xy_sum * inv_len;        //sum( X[n] * Y[n] )
            T XZ = xz_sum * inv_len;        //sum( X[n] * Z[n] )
            T YZ = yz_sum * inv_len;        //sum( Y[n] * Z[n] )
            T X2Y = x2y_sum * inv_len;    //sum( X[n]^2 * Y[n] )
            T X2Z = x2z_sum * inv_len;    //sum( X[n]^2 * Z[n] )
            T Y2X = y2x_sum * inv_len;    //sum( Y[n]^2 * X[n] )
            T Y2Z = y2z_sum * inv_len;    //sum( Y[n]^2 * Z[n] )
            T Z2X = z2x_sum * inv_len;    //sum( Z[n]^2 * X[n] )
            T Z2Y = z2y_sum * inv_len;    //sum( Z[n]^2 * Y[n] )

            //Reduction of multiplications
            T F0 = x_sum2 + y_sum2 + z_sum2;
            T F1 =  static_cast<T>(0.5f) * F0;
            T F2 = -static_cast<T>(8) * (x_sum3 + Y2X + Z2X);
            T F3 = -static_cast<T>(8) * (X2Y + y_sum3 + Z2Y);
            T F4 = -static_cast<T>(8) * (X2Z + Y2Z + z_sum3);

            //Set initial conditions:
            T A = x_sum;
            T B = y_sum;
            T C = z_sum;

            //First iteration computation:
            T A2 = A * A;
            T B2 = B * B;
            T C2 = C * C;
            T QS = A2 + B2 + C2;
            T QB = -2 * (A * x_sum + B * y_sum + C * z_sum);

            //Set initial conditions:
            T Rsq = F0 + QB + QS;

            //First iteration computation:
            T Q0 = static_cast<T>(0.5f) * (QS - Rsq);
            T Q1 = F1 + Q0;
            T Q2 = 8 * (QS - Rsq + QB + F0);
            T aA, aB, aC, nA, nB, nC, dA, dB, dC;

            //Iterate N times, ignore stop condition.
            size_t n = 0;

            while (n < max_iterations) {
                n++;

                //Compute denominator:
                aA = Q2 + 16 * (A2 - 2 * A * x_sum + x_sum2);
                aB = Q2 + 16 * (B2 - 2 * B * y_sum + y_sum2);
                aC = Q2 + 16 * (C2 - 2 * C * z_sum + z_sum2);
                aA = (aA == 0) ? 1 : aA;
                aB = (aB == 0) ? 1 : aB;
                aC = (aC == 0) ? 1 : aC;

                //Compute next iteration
                nA = A - ((F2 + 16 * (B * XY + C * XZ + x_sum * (-A2 - Q0) + A * (x_sum2 + Q1 - C * z_sum - B * y_sum))) / aA);
                nB = B - ((F3 + 16 * (A * XY + C * YZ + y_sum * (-B2 - Q0) + B * (y_sum2 + Q1 - A * x_sum - C * z_sum))) / aB);
                nC = C - ((F4 + 16 * (A * XZ + B * YZ + z_sum * (-C2 - Q0) + C * (z_sum2 + Q1 - A * x_sum - B * y_sum))) / aC);

                //Check for stop condition
                dA = (nA - A);
                dB = (nB - B);
                dC = (nC - C);

                if ((dA * dA + dB * dB + dC * dC) <= delta) { break; }

                //Compute next iteration's values
                A = nA;
                B = nB;
                C = nC;
                A2 = A * A;
                B2 = B * B;
                C2 = C * C;
                QS = A2 + B2 + C2;
                QB = -2 * (A * x_sum + B * y_sum + C * z_sum);
                Rsq = F0 + QB + QS;
                Q0 = 0.5f * (QS - Rsq);
                Q1 = F1 + Q0;
                Q2 = 8 * (QS - Rsq + QB + F0);
            }

            return FitResult{
                .center = math::Vec3<T>(A, B, C),
                .radius = math::sqrt(Rsq)
            };
        }
    };
};


}