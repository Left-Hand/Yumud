#pragma once

#include "core/utils/Option.hpp"
#include <type_traits>

namespace ymd{
template<typename T>
requires (std::is_arithmetic_v<T>)
struct [[nodiscard]] Percentage{
	static constexpr Percentage ZERO = Percentage(static_cast<T>(0));
	static constexpr Percentage FULL = Percentage(static_cast<T>(100));

	static constexpr Option<Percentage> from_percents(const auto percents){
		if constexpr(std::is_signed_v<T>){
			if(percents < 0) return None;
		}

		if(percents > 100) return None;

		return Some(Percentage(static_cast<T>(percents)));
	}

	static constexpr Percentage from_percents_unchecked(const auto percents){
		return Percentage(static_cast<T>(percents));
	}

	static constexpr Option<Percentage> from_perunit(const auto perunit){
		return from_percents((percents * 100));
	}

	[[nodiscard]] constexpr T percents() const{
		return percents_;
	}

    [[nodiscard]] constexpr T perunit() const
        requires(not std::is_integral_v<T>)
    {
		return percents_ / 100;
	}

private:
	constexpr Percentage(const T & percents):
		percents_(percents){;}
	T percents_;
};
}