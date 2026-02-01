#pragma once

#include "core/math/real.hpp"
#include "algebra/vectors/vec2.hpp"

namespace ymd::intp{

struct [[nodiscard]] CosineInterpolation final{
public:
    constexpr iq16 forward(iq16 x) const{
        iq16 x2 = x*x;
        iq16 x4 = x2*x2;
        iq16 x6 = x4*x2;
        
        constexpr iq16 fa = iq16( 4.0/9.0);
        constexpr iq16 fb = iq16(-17.0/9.0);
        constexpr iq16 fc = iq16(22.0/9.0);
        
        return fa*x6 + fb*x4 + fc*x2;
    }
};


struct [[nodiscard]] SeatInterpolation final{
public:

    static constexpr double epsilon = 0.001;
    static constexpr iq16 MIM_PARAM_A = iq16(0.0 + epsilon);
    static constexpr iq16 MAX_PARAM_A = iq16(1.0 - epsilon);
    static constexpr iq16 MIN_PARAM_A = iq16(0.0);
    static constexpr iq16 MAX_PARAM_B = iq16(1.0);
    static constexpr std::tuple<iq16, iq16> get_ab(const math::Vec2<iq16> & handle){

        auto [a,b] = handle;

        a = MIN(MAX_PARAM_A, MAX(MIM_PARAM_A, a));  
        b = MIN(MAX_PARAM_B, MAX(MIN_PARAM_A, b)); 
        return std::make_tuple(a,b);
    }
    iq16 a;
    iq16 b;
public:
    constexpr explicit SeatInterpolation(const math::Vec2<iq16> & handle){
        std::tie(a,b) = get_ab(handle);
    }

    constexpr iq16 forward(const iq16 x) const{
        iq16 y = 0;
        if (x <= a){
            y = b - b*math::cubic(1-x/a);
        } else {
            y = b + (1-b)*math::cubic((x-a)/(1-a));
        }
        return y;
    }
};

struct [[nodiscard]] SeatLineInterpolation final{
public:
    iq16 a;
    iq16 b;
    constexpr iq16 forward(const iq16 x) const{
        iq16 y = 0;
        if (x<=a){
            y = b*x + (1-b)*a*(1-math::cubic(1-x/a));
        } else {
            y = b*x + (1-b)*(a + (1-a)*math::cubic((x-a)/(1-a)));
        }
        return y;
    }
};

struct [[nodiscard]] SeatOddInterpolation final{
    iq16 a;
    iq16 b;
    // p(2 * n + 1)
    int p;
public:
    constexpr iq16 forward(const iq16 x) const{
        // auto [a,b] = get_ab(handle);
        // int p = 2*n + 1;
        iq16 y = 0;
        if (x <= a){
            y = b - b*math::powfi(1-x/a, p);
        } else {
            y = b + (1-b)*math::powfi((x-a)/(1-a), p);
        }
        return y;

    }
};


struct [[nodiscard]] SymmetricInterpolation final{
    iq16 a;
    iq16 b;
    int _n;

    constexpr iq16 forward(const iq16 x) const{
        iq16 y = 0;
        if (+_n%2 == 0){ 
            // even polynomial
            if (x<=iq16(0.5)){
            y = math::powfi(2*x, _n)/2;
            } else {
            y = 1 - math::powfi(2*(x-1), _n)/2;
            }
        } 
        
        else { 
            // odd polynomial
            if (x<=iq16(0.5)){
            y = math::powfi(2*x, _n)/2;
            // y = (2*x, _n)/2;
            } else {
            y = 1 + math::powfi(2*(x-1), _n)/2;
            }
        }

        return y;
    }
};

struct [[nodiscard]] QuadraticSeatInterpolation final{
    iq16 a;
    iq16 b;
    int _n;
public:
    constexpr iq16 forward(const iq16 x) const {
        iq16 A = (1-b)/(1-a) - (b/a);
        iq16 B = (A*(a*a)-b)/a;
        iq16 y = A*(x*x) - B*(x);
        y = MIN(1,MAX(0,y)); 
        
        return y;
    }
};



}