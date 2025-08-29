#pragma once

#include "core/math/real.hpp"
#include "core/stream/ostream.hpp"

namespace ymd{

template<typename T>
struct Angle{
	static_assert(not std::is_integral_v<T>, "T must be not integral");

    static constexpr Angle<T> ZERO = 
		Angle<T>::from_turns(static_cast<T>(0));

    static constexpr Angle<T> LAP = 
		Angle<T>::from_turns(static_cast<T>(1));

    static constexpr Angle<T> NEG_LAP = 
		Angle<T>::from_turns(static_cast<T>(-1));

    static constexpr Angle<T> HALF_LAP = 
		Angle<T>::from_turns(static_cast<T>(0.5));

    static constexpr Angle<T> NEG_HALF_LAP = 
		Angle<T>::from_turns(static_cast<T>(-0.5));


	static constexpr Angle<T> TRIS_LAP = 
		Angle<T>::from_turns(static_cast<T>(1.0 / 3));

	static constexpr Angle<T> NEG_TRIS_LAP = 
		Angle<T>::from_turns(static_cast<T>(1.0 / 3));

    static constexpr Angle<T> QUARTER_LAP = 
		Angle<T>::from_turns(static_cast<T>(0.25));


    static constexpr Angle<T> NEG_QUARTER_LAP = 
		Angle<T>::from_turns(static_cast<T>(-0.25));

    static constexpr Angle<T> THREE_QUARTERS = 
		Angle<T>::from_turns(static_cast<T>(0.75));

    static constexpr Angle<T> NEG_THREE_QUARTERS = 
		Angle<T>::from_turns(static_cast<T>(-0.75));


	static constexpr Angle<T> DEG_0 = Angle<T>::from_degrees(static_cast<T>(0));
	static constexpr Angle<T> DEG_30 = Angle<T>::from_degrees(static_cast<T>(30));
	static constexpr Angle<T> DEG_60 = Angle<T>::from_degrees(static_cast<T>(60));
	static constexpr Angle<T> DEG_90 = Angle<T>::from_degrees(static_cast<T>(90));
	static constexpr Angle<T> DEG_120 = Angle<T>::from_degrees(static_cast<T>(120));
	static constexpr Angle<T> DEG_180 = Angle<T>::from_degrees(static_cast<T>(180));
	static constexpr Angle<T> DEG_270 = Angle<T>::from_degrees(static_cast<T>(270));

	template<typename U>
	requires std::is_floating_point_v<U>
	static constexpr Angle from_degrees(const U degrees){
		if constexpr(is_fixed_point_v<T>){
			constexpr U INV_360 = static_cast<U>(1.0 / 360.0);
			// Angle ret;
			return make_angle_from_turns(T::from(static_cast<float>(degrees * INV_360)));
			// return ret;
		}else{
			constexpr U INV_360 = static_cast<U>(1.0 / 360.0);
			// Angle ret;
			// ret.turns_ = static_cast<T>(degrees * INV_360);
			// return ret;

			return make_angle_from_turns(static_cast<T>(degrees * INV_360));
		}
	}

	template<typename U>
	requires (std::is_integral_v<U> || is_fixed_point_v<U>)
	static constexpr Angle from_degrees(const U degrees){
		if constexpr(std::is_integral_v<U>){ 
			constexpr T INV_360 = static_cast<T>(1.0 / 360.0);
			return make_angle_from_turns(degrees * INV_360);
		}else{
			return make_angle_from_turns(static_cast<T>(degrees / 360));
		}
	}

	static constexpr Angle from_radians(const T radians){
		constexpr T INV_TAU = static_cast<T>(1.0 / (2.0 * 3.1415926535897932384626433832795));
		return make_angle_from_turns(radians * INV_TAU);
	}

	static constexpr Angle from_turns(const T turns){
		return make_angle_from_turns(turns);
	}

	template<typename U>
	constexpr Angle<T> operator = (const Angle<U> & rhs){
		return Angle<T>::from_turns(static_cast<T>(rhs.turns_));
	}

	constexpr T to_degrees() const{
		return turns_ * 360;
	}

	constexpr T to_radians() const{
		return turns_ * static_cast<T>(TAU);
	}

	constexpr T to_turns() const{
		return turns_;
	}

    constexpr std::array<T, 2> sincos() const {
		if constexpr(
			std::is_same_v<
				std::array<T, 2>, 
				decltype(sincospu(turns_))
			>
		){
			return sincospu(turns_);
		}
		const auto [s,c] = sincospu(turns_);
		return {static_cast<T>(s), static_cast<T>(c)};
    }

	constexpr T sin() const{
		return sinpu(turns_ );
	}

	constexpr T cos() const{
		return cospu(turns_ );
	}

	constexpr T tan() const{
		return tan(to_radians());
	}

	constexpr Angle forward_angle_to(const Angle& target) const {
		// 计算正向（逆时针）到达目标角度需要转动的角度
		T diff = target.turns_ - turns_;
		if (diff < 0) diff += 1; // 确保结果在 [0, 1) 范围内
		return make_angle_from_turns(diff);
	}

	constexpr Angle backward_angle_to(const Angle& target) const {
		// 计算反向（顺时针）到达目标角度需要转动的角度
		T diff = turns_ - target.turns_;
		if (diff < 0) diff += 1; // 确保结果在 [0, 1) 范围内
		return make_angle_from_turns(diff);
	}

	constexpr Angle shortest_angle_to(const Angle& target) const {
		// 计算到目标角度的最短路径角度（返回绝对值最小的角度，符号表示方向）
		T diff = target.turns_ - turns_;
		
		// 归一化到 [-0.5, 0.5) 范围
		if (diff > 0.5) diff -= 1;
		else if (diff < -0.5) diff += 1;
		
		return make_angle_from_turns(diff);
	}

	constexpr Angle operator+(const Angle & rhs) const{
		return make_angle_from_turns(turns_ + rhs.turns_);
	}

	constexpr Angle operator-(const Angle & rhs) const{
		return make_angle_from_turns(turns_ - rhs.turns_);
	}

	constexpr Angle operator -() const{
		return make_angle_from_turns(-turns_);
	} 

	constexpr Angle operator +() const {
		return make_angle_from_turns(+turns_);
	}

	constexpr Angle operator*(const T rhs) const{
		return make_angle_from_turns(turns_ * rhs);
	}

	constexpr Angle operator/(const T rhs) const{
		return make_angle_from_turns(turns_ / rhs);
	}

	template<typename U>
	constexpr Angle operator*(const U rhs) const{
		return make_angle_from_turns(turns_ * rhs);
	}
	
	template<typename U>
	constexpr Angle operator/(const U rhs) const{
		return make_angle_from_turns(turns_ / rhs);
	}

	template<typename U>
	constexpr Angle mod(const Angle<U> rhs) const{
		return make_angle_from_turns(fposmod(turns_, static_cast<T>(rhs.turns_)));
	}

	template<typename U>
	friend constexpr Angle operator*(const U lhs, const Angle & rhs) {
		return make_angle_from_turns(lhs * rhs.turns_);
	}


	#define DEF_COMPARISON_OPERATOR(OP) \
	constexpr bool operator OP(const Angle & rhs) const{ \
		return turns_ OP rhs.turns_; \
	}

	DEF_COMPARISON_OPERATOR(==)
	DEF_COMPARISON_OPERATOR(!=)
	DEF_COMPARISON_OPERATOR(<)
	DEF_COMPARISON_OPERATOR(>)
	DEF_COMPARISON_OPERATOR(<=)
	DEF_COMPARISON_OPERATOR(>=)

	#undef DEF_COMPARISON_OPERATOR

	constexpr Angle<T> abs() const {
		return make_angle_from_turns(ABS(turns_));
	}

	constexpr Angle<T> normalized() const {
		return make_angle_from_turns(frac(turns_));
	}

	constexpr bool is_equal_approx(const Angle<T> & other) const {
		return ymd::is_equal_approx(turns_, other.turns_);
	}

	constexpr Angle<T> lerp(const Angle<T> & other, const T ratio) const {
		return make_angle_from_turns(ymd::lerp(turns_, other.turns_, ratio));
	}

	constexpr bool is_positive() const {
		return turns_ >= 0;
	}

	constexpr bool is_negative() const {
		return turns_ < 0;
	}

	constexpr bool is_wrapped() const {
		return int(turns_) == 0;
	}

	friend OutputStream & operator <<(OutputStream & os, const Angle & self){
		// return os << self.to_degrees() << '\'';
		
		return os << static_cast<q16>(self.to_turns()) * 360 << '\'';
	}
// private:
public:
	T turns_;

	static constexpr Angle make_angle_from_turns(auto turns){
		return Angle{.turns_ = turns};
	}
};

template<typename T>
static constexpr bool is_equal_approx(const Angle<T> a, const Angle<T> b){
	return a.is_equal_approx(b);
}

consteval Angle<real_t> operator"" _deg(long double x){
    return Angle<real_t>::from_degrees(x);
}

consteval Angle<real_t> operator"" _deg(uint64_t x){
    return Angle<real_t>::from_degrees(x);
}

consteval Angle<real_t> operator"" _rad(long double x){
    return Angle<real_t>::from_radians(static_cast<real_t>(x));
}

consteval Angle<real_t> operator"" _rad(uint64_t x){
    return Angle<real_t>::from_radians(static_cast<real_t>(x));
}

consteval Angle<real_t> operator"" _turn(long double x){
    return Angle<real_t>::from_turns(static_cast<real_t>(x));
}

consteval Angle<real_t> operator"" _turn(uint64_t x){
    return Angle<real_t>::from_turns(static_cast<real_t>(x));
}

}