#pragma once

#include "../constexprmath/ConstexprMath.hpp"
#include "core/math/real.hpp"


template<typename T, size_t N>
class Cordic{
protected:
    using cem = ConstexprMath;

	struct Coord{
		T x;
		T y;
	};

	static constexpr std::array<_iq, N> sine{[]{
        std::array<_iq, N> temp = {};
        for(size_t i = 0; i < N; ++i) {
            temp[i] = _iq(cem::sin(PI / 4 * cem::pow(2.0, double(-i)))*(1 << GLOBAL_Q));
        }
        return temp;
    }()};

	static constexpr std::array<_iq, N> cosine{[]{
        std::array<_iq, N> temp = {};
        for(size_t i = 0; i < N; ++i) {
            temp[i] = _iq(cem::cos(PI / 4 * cem::pow(2.0, double(-i)))*(1 << GLOBAL_Q));
        }
        return temp;
    }()};

	constexpr Coord sincosu(const T _x) const{
		T unit = T(PI / 4);
		T x = fmod(_x, PI);
		Coord coord = Coord{T(1), T(0)};

		for(uint8_t i = 0; i < N; i++){
			if(x > unit){
				coord = Coord{
					coord.x * iq16(cosine[i]) - coord.y * iq16(sine[i]),
					coord.y * iq16(cosine[i]) + coord.x * iq16(sine[i])
				};
				x -= unit;
			}
			unit /= 2;
		}

		return coord;
	}

	constexpr Coord atan2squu(const T _y, const T _x) const{
		Coord coord = Coord{_x, _y};
		T angleSum = T(0);
		T angle = T(PI / 4);

		for(uint8_t i = 0; i < N; i++){
			if(coord.y > 0){
				coord = Coord{
					coord.x * iq16(cosine[i]) + coord.y * iq16(sine[i]),
					coord.y * iq16(cosine[i]) - coord.x * iq16(sine[i])
				};
				angleSum += angle;
			}
			else{
				coord = Coord{
					coord.x * iq16(cosine[i]) - coord.y * iq16(sine[i]),
					coord.y * iq16(cosine[i]) + coord.x * iq16(sine[i])
				};
				angleSum -= angle;
			}
			angle /= 2;
		}
		return Coord{coord.x, angleSum};
	}

	constexpr Coord atan2squ(const T y, const T x) const{

		if(x >= 0){
			Coord ret = atan2squu(abs(y), x);
			T atan2_abs = ret.y;
			T squ_abs = ret.x;
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

	constexpr T squ(const T y, const T x) const{
		return atan2squ(y,x).x;
	}

	constexpr T atan2(const T y, const T x) const{
		return atan2squ(y, x).y;
	}

	constexpr T asin(const T x) const {
		if (x <= -1) return -T(PI / 2);
		if (x >= 1) return T(PI / 2);
		return atan2(x, sqrt(1 - x * x));
	}

	constexpr T acos(const T x) const{
		if (x <= -1) return T(PI);
		if (x >= 1) return T(0);
		return atan2(sqrt(1 - x * x), x);
	}

	constexpr T sin(const T x) const{
		T ret = sincosu(x).y;
		return x > 0 ? ret : -ret;
	}

	constexpr T cos(const T x){
		return sincosu(abs(x)).x;
	}

//	T tan(const T x){
//		Coord coord
//	}
};

