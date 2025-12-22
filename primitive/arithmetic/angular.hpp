#pragma once

#include "core/math/realmath.hpp"
#include "core/stream/ostream.hpp"

namespace ymd{

template<typename T>
struct Rotation2;

//描述了类型安全的角度
template<typename T>
struct [[nodiscard]] Angular{
	static_assert(not std::is_integral_v<T>, "T must be not integral because the storage is turns");

	using ST = std::make_signed<T>;
	using UT = std::make_unsigned<T>;

    static constexpr Angular<T> ZERO = 
		Angular<T>::from_turns(static_cast<T>(0));

    static constexpr Angular<T> ONE = 
		Angular<T>::from_turns(static_cast<T>(1));

    static constexpr Angular<T> NEG_ONE = 
		Angular<T>::from_turns(static_cast<T>(-1));

    static constexpr Angular<T> HALF = 
		Angular<T>::from_turns(static_cast<T>(0.5));

    static constexpr Angular<T> NEG_HALF = 
		Angular<T>::from_turns(static_cast<T>(-0.5));


	static constexpr Angular<T> TRIS = 
		Angular<T>::from_turns(static_cast<T>(1.0 / 3));

	static constexpr Angular<T> NEG_TRIS = 
		Angular<T>::from_turns(static_cast<T>(-1.0 / 3));

    static constexpr Angular<T> QUARTER = 
		Angular<T>::from_turns(static_cast<T>(0.25));


    static constexpr Angular<T> NEG_QUARTER = 
		Angular<T>::from_turns(static_cast<T>(-0.25));

    static constexpr Angular<T> THREE_QUARTERS = 
		Angular<T>::from_turns(static_cast<T>(0.75));

    static constexpr Angular<T> NEG_THREE_QUARTERS = 
		Angular<T>::from_turns(static_cast<T>(-0.75));


	static constexpr Angular<T> DEG_0 	= Angular<T>::from_degrees(static_cast<T>(0));
	static constexpr Angular<T> DEG_30 	= Angular<T>::from_degrees(static_cast<T>(30));
	static constexpr Angular<T> DEG_60 	= Angular<T>::from_degrees(static_cast<T>(60));
	static constexpr Angular<T> DEG_90 	= Angular<T>::from_degrees(static_cast<T>(90));
	static constexpr Angular<T> DEG_120 = Angular<T>::from_degrees(static_cast<T>(120));
	static constexpr Angular<T> DEG_180 = Angular<T>::from_degrees(static_cast<T>(180));
	static constexpr Angular<T> DEG_270 = Angular<T>::from_degrees(static_cast<T>(270));

	template<typename U>
	requires std::is_floating_point_v<U>
	static constexpr Angular from_degrees(const U degrees){
		if constexpr(tmp::is_fixed_point_v<T>){
			constexpr U INV_360 = static_cast<U>(1.0 / 360.0);
			return make_angle_from_turns(T::from(static_cast<float>(degrees * INV_360)));
		}else{
			constexpr U INV_360 = static_cast<U>(1.0 / 360.0);
			return make_angle_from_turns(static_cast<T>(degrees * INV_360));
		}
	}

	template<typename D>
	requires (std::is_integral_v<D>)
	static constexpr Angular from_degrees(const D degrees){
		static_assert(sizeof(D) <= sizeof(T));
		constexpr T INV_360 = static_cast<T>(1.0 / 360.0);
		const auto turns = INV_360 * degrees;
		return make_angle_from_turns(static_cast<T>(turns));
	}

	template<typename U>
	requires (tmp::is_fixed_point_v<U>)
	static constexpr Angular from_degrees(const U degrees){
		return make_angle_from_turns(static_cast<T>(degrees / 360));
	}

	static constexpr Angular from_radians(const T radian){
		constexpr T INV_TAU = static_cast<T>(1.0 / (2.0 * 3.1415926535897932384626433832795));
		return make_angle_from_turns(radian * INV_TAU);
	}

	static constexpr Angular from_turns(const T turns){
		return make_angle_from_turns(turns);
	}

	static constexpr Angular from_atan2(const T y, const T x){
		return make_angle_from_turns(atan2pu(y, x));
	}

	template<typename U>
	constexpr Angular<T> & operator = (const Angular<U> & rhs){
		*this = Angular<T>::from_turns(static_cast<T>(rhs.turns_));
		return *this;
	}

	[[nodiscard]] constexpr T to_degrees() const{
		return turns_ * 360;
	}

	[[nodiscard]] constexpr T to_radians() const{
		return turns_ * static_cast<T>(TAU);
	}

	[[nodiscard]] constexpr T to_turns() const{
		return turns_;
	}

	constexpr void set_radians(const T radians){
		*this = Angular<T>::from_radians(radians);
	}

	constexpr void set_turns(const T turns){
		*this = Angular<T>::from_turns(turns);
	}

	constexpr void set_degrees(const T degrees){
		*this = Angular<T>::from_degrees(degrees);
	}

    [[nodiscard]] constexpr auto sincos() const {
		if constexpr(
			std::is_same_v<
				std::array<T, 2>, 
				decltype(math::sincospu(turns_))
			>
		){
			return math::sincospu(turns_);
		}else{
			return math::sincospu(turns_);
		}
    }

	[[nodiscard]] constexpr auto sin() const{
		return math::sinpu(turns_);
	}

	[[nodiscard]] constexpr auto cos() const{
		return cospu(turns_ );
	}

	[[nodiscard]] constexpr auto tan() const{
		return tan(to_radians());
	}

	constexpr Angular forward_angle_to(const Angular& target) const {
		// 计算正向（逆时针）到达目标角度需要转动的角度
		T diff = target.turns_ - turns_;
		if (diff < 0) diff += 1; // 确保结果在 [0, 1) 范围内
		return make_angle_from_turns(diff);
	}

	constexpr Angular backward_angle_to(const Angular& target) const {
		// 计算反向（顺时针）到达目标角度需要转动的角度
		T diff = turns_ - target.turns_;
		if (diff < 0) diff += 1; // 确保结果在 [0, 1) 范围内
		return make_angle_from_turns(diff);
	}

	constexpr Angular shortest_angle_to(const Angular& target) const {
		// 计算到目标角度的最短路径角度（返回绝对值最小的角度，符号表示方向）
		T diff = target.turns_ - turns_;
		
		// 归一化到 [-0.5, 0.5) 范围
		if (diff > 0.5) diff -= 1;
		else if (diff < -0.5) diff += 1;
		
		return make_angle_from_turns(diff);
	}

	constexpr Angular operator + (const Angular & rhs) const{
		return make_angle_from_turns(turns_ + rhs.turns_);
	}

	constexpr Angular operator -(const Angular & rhs) const{
		return make_angle_from_turns(turns_ - rhs.turns_);
	}

	constexpr Angular operator -() 
	const requires(std::is_signed_v<T>){
		return make_angle_from_turns(-turns_);
	} 

	constexpr Angular operator +() const {
		return make_angle_from_turns(+turns_);
	}

	template<typename U>
	constexpr Angular operator*(const U rhs) const{
		return make_angle_from_turns(turns_ * rhs);
	}

	template<typename U>
	constexpr Angular operator/(const U rhs) const{
		return make_angle_from_turns(turns_ / rhs);
	}


	template<typename U>
	constexpr Angular mod(const Angular<U> rhs) const{
		return make_angle_from_turns(fposmod(turns_, static_cast<T>(rhs.turns_)));
	}

	template<typename U>
	friend constexpr Angular operator*(const U lhs, const Angular & rhs) {
		return make_angle_from_turns(lhs * rhs.turns_);
	}


	#define DEF_COMPARISON_OPERATOR(OP) \
	constexpr bool operator OP(const Angular & rhs) const{ \
		return turns_ OP rhs.turns_; \
	}

	DEF_COMPARISON_OPERATOR(==)
	DEF_COMPARISON_OPERATOR(!=)
	DEF_COMPARISON_OPERATOR(<)
	DEF_COMPARISON_OPERATOR(>)
	DEF_COMPARISON_OPERATOR(<=)
	DEF_COMPARISON_OPERATOR(>=)

	#undef DEF_COMPARISON_OPERATOR

	[[nodiscard]] constexpr Angular<T> abs() const {
		return make_angle_from_turns(ABS(turns_));
	}

	[[nodiscard]] constexpr Angular<ST> to_signed() const {
		if constexpr(std::is_signed_v<T>)
			return *this;
		return Angular<ST>::make_angle_from_turns(static_cast<ST>(turns_));
	}

	[[nodiscard]] constexpr Angular<UT> to_unsigned() const {
		if constexpr(std::is_unsigned_v<T>)
			return *this;
		return Angular<ST>::make_angle_from_turns(static_cast<UT>(turns_));
	}

	[[nodiscard]] constexpr Angular<T> signed_normalized() 
	const requires(std::is_signed_v<T>){
		//归一化到[-0.5, 0.5)之间
		return make_angle_from_turns(turns_ - int(math::floor(turns_ + static_cast<T>(0.5))));
	}

	[[nodiscard]] constexpr Angular<T> unsigned_normalized() const{
		//归一化到[0, 1)之间
		return make_angle_from_turns(math::frac(turns_));
	}

	[[nodiscard]] constexpr bool is_orthogonal_with(const Angular<T> & other, const Angular<T> & eps) const {
		const auto norm_self = this->unsigned_normalized();
		const auto comp1 = (other + Angular<T>::from_turns(static_cast<T>(0.25))
			).unsigned_normalized();
		const auto comp2 = (other + Angular<T>::from_turns(static_cast<T>(0.75))
			).unsigned_normalized();
		if(norm_self.is_equal_approx(comp1, eps)) return true;
		if(norm_self.is_equal_approx(comp2, eps)) return true;
		return false;
	}

	[[nodiscard]] constexpr bool is_equal_approx(const Angular<T> & other, const Angular<T> & eps) const {
		return math::is_equal_approx(turns_, other.turns_, eps.turns_);
	}

	[[nodiscard]] constexpr Angular<T> lerp(const Angular<T> & other, const T ratio) const {
		return make_angle_from_turns(math::lerp(turns_, other.turns_, ratio));
	}

	[[nodiscard]] constexpr bool is_positive() const {
		if constexpr(std::is_unsigned_v<T>)
			return true;
		else 
			return turns_ > 0;
	}

	[[nodiscard]] constexpr bool is_negative() const {
		if constexpr(std::is_unsigned_v<T>)
			return false;
		else
			return turns_ < 0;
	}

	template<typename U>
	[[nodiscard]] constexpr Angular<U> cast_inner() const {
		return Angular<U>::make_angle_from_turns(static_cast<U>(turns_));
	}

	[[nodiscard]] constexpr Rotation2<T> to_rotation() const {
		return Rotation2<T>::from_angle(*this);
	}

	friend OutputStream & operator <<(OutputStream & os, const Angular & self){
		// return os << self.to_degrees() << '\'';
		if constexpr (tmp::is_fixed_point_v<T>)
			return os << static_cast<iq16>(self.to_turns()) * 360 << '\'';
		else
			return os << self.to_turns() * 360 << '\'';
	}
public:
	T turns_;

	static constexpr Angular make_angle_from_turns(auto turns){
		return Angular{.turns_ = static_cast<T>(turns)};
		// return Angular{.turns_ = 0};
	}

	template<typename P>
	friend class Angular;
};

template<typename T>
static constexpr bool is_equal_approx(const Angular<T> a, const Angular<T> b){
	return a.is_equal_approx(b);
}

consteval Angular<real_t> operator"" _deg(long double x){
    return Angular<real_t>::from_degrees(x);
}

//字面量只能被uint64_t覆写 内部转换到uint32_t
consteval Angular<real_t> operator"" _deg(uint64_t x){
    return Angular<real_t>::from_degrees(static_cast<uint32_t>(x));
}

consteval Angular<real_t> operator"" _rad(long double x){
    return Angular<real_t>::from_radians(static_cast<real_t>(x));
}

consteval Angular<real_t> operator"" _rad(uint64_t x){
    return Angular<real_t>::from_radians(static_cast<real_t>(x));
}

consteval Angular<real_t> operator"" _turn(long double x){
    return Angular<real_t>::from_turns(static_cast<real_t>(x));
}

consteval Angular<real_t> operator"" _turn(uint64_t x){
    return Angular<real_t>::from_turns(static_cast<real_t>(x));
}

}