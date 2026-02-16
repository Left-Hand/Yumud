#pragma once

#include "core/math/iq/fixed_t.hpp"

namespace ymd::str{

[[nodiscard]] char * fmtnum_i32(char * p_str, const int32_t value, uint8_t radix);
[[nodiscard]] char * fmtnum_u32(char * p_str, const uint32_t value, uint8_t radix);
[[nodiscard]] char * fmtnum_u64(char * p_str, const uint64_t value, uint8_t radix);
[[nodiscard]] char * fmtnum_i64(char * p_str, const int64_t value, uint8_t radix);
[[nodiscard]] char * fmtnum_f32(char * p_str, const float value, uint8_t precsion);

[[nodiscard]] char * _fmtnum_signed_fixed(char * p_str, 
    const uint32_t value_bits, uint8_t precsion, const uint8_t Q);

[[nodiscard]] char * _fmtnum_unsigned_fixed(char * p_str, 
    const uint32_t value_bits, uint8_t precsion, const uint8_t Q);

template<typename D>
requires(sizeof(D) <= 4)
[[nodiscard]] char * fmtnum_fixed(
    char * const str, 
    const D bits, 
    uint8_t precsion, 
    const size_t Q
){
	using size_aligned_t = std::conditional_t<std::is_signed_v<D>, int32_t, uint32_t>;
	static_assert(sizeof(size_aligned_t) == sizeof(D));
	if constexpr(std::is_signed_v<D>)
		return _fmtnum_signed_fixed(str, int32_t(bits), precsion, Q);
	else
		return _fmtnum_unsigned_fixed(str, uint32_t(bits), precsion, Q);
}


}