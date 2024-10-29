#ifndef __CORDIC_HPP__

#define __CORDIC_HPP__

#include "../constexprmath/ConstexprMath.hpp"
#include "sys/math/real.hpp"


template<typename real, int size>
class Cordic{
protected:
    using cem = ConstexprMath;

	struct Coord{
		real x;
		real y;
	};

	scexpr std::array<_iq, size> sine{[]{
        std::array<_iq, size> temp = {};
        for(int i = 0; i < size; ++i) {
            temp[i] = _iq(cem::sin(PI / 4 * cem::pow(2.0, double(-i)))*(1 << GLOBAL_Q));
        }
        return temp;
    }()};

	scexpr std::array<_iq, size> cosine{[]{
        std::array<_iq, size> temp = {};
        for(int i = 0; i < size; ++i) {
            temp[i] = _iq(cem::cos(PI / 4 * cem::pow(2.0, double(-i)))*(1 << GLOBAL_Q));
        }
        return temp;
    }()};

	Coord sincosu(const real & _x) const{
		real unit = real(PI / 4);
		real x = fmod(_x, PI);
		Coord coord = Coord{real(1), real(0)};

		for(uint8_t i = 0; i < size; i++){
			if(x > unit){
				coord = Coord{
					coord.x * real_t(cosine[i]) - coord.y * real_t(sine[i]),
					coord.y * real_t(cosine[i]) + coord.x * real_t(sine[i])
				};
				x -= unit;
			}
			unit /= 2;
		}

		return coord;
	}

	Coord atan2squu(const real & _y, const real & _x) const{
		Coord coord = Coord{_x, _y};
		real angleSum = real(0);
		real angle = real(PI / 4);

		for(uint8_t i = 0; i < size; i++){
			if(coord.y > 0){
				coord = Coord{
					coord.x * real_t(cosine[i]) + coord.y * real_t(sine[i]),
					coord.y * real_t(cosine[i]) - coord.x * real_t(sine[i])
				};
				angleSum += angle;
			}
			else{
				coord = Coord{
					coord.x * real_t(cosine[i]) - coord.y * real_t(sine[i]),
					coord.y * real_t(cosine[i]) + coord.x * real_t(sine[i])
				};
				angleSum -= angle;
			}
			angle /= 2;
		}
		return Coord{coord.x, angleSum};
	}

	Coord atan2squ(const real & y, const real & x) const{

		if(x >= 0){
			Coord ret = atan2squu(abs(y), x);
			real atan2_abs = ret.y;
			real squ_abs = ret.x;
			return Coord{squ_abs, atan2_abs > 0 ? atan2_abs : -atan2_abs};
		}else{
			if(y > 0){
				Coord ret = atan2squu(-x, y);
				return Coord{ret.x, ret.y + PI / 2};
			}else{
				Coord ret = atan2squu(-x, -y);
				return Coord{ret.x, - PI / 2 - ret.y};
			}
		}
	}
public:
	consteval Cordic(){;}

	real squ(const real & y, const real & x) const{
		return atan2squ(y,x).x;
	}
	real atan2(const real & y, const real & x) const{
		return atan2squ(y, x).y;
	}

	real asin(const real & x) const {
		if (x <= -1) return -real(PI / 2);
		if (x >= 1) return real(PI / 2);
		return atan2(x, sqrt(1 - x * x));
	}

	real acos(const real & x) const{
		if (x <= -1) return real(PI);
		if (x >= 1) return real(0);
		return atan2(sqrt(1 - x * x), x);
	}

	real sin(const real & x) const{
		real ret = sincosu(x).y;
		return x > 0 ? ret : -ret;
	}

	real cos(const real & x){
		return sincosu(abs(x)).x;
	}

//	real tan(const real & x){
//		Coord coord
//	}
};

#endif