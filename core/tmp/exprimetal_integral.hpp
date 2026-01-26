#pragma once

#include <cstdint>

namespace ymd::tmp{


namespace details{
template<typename T>
struct _extended_unsigned;
template<> struct _extended_unsigned<uint8_t>{
	using type = uint32_t;
};
template<> struct _extended_unsigned<uint16_t>{
	using type = uint32_t;
};
template<> struct _extended_unsigned<uint32_t>{
	using type = uint64_t;
};
template<> struct _extended_unsigned<int8_t>{
	using type = uint32_t;
};
template<> struct _extended_unsigned<int16_t>{
	using type = uint32_t;
};
template<> struct _extended_unsigned<int32_t>{
	using type = uint32_t;
};
template<> struct _extended_unsigned<int>{
	using type = uint32_t;
};

template<typename T>
struct _extended_signed;
template<> struct _extended_signed<uint8_t>{
	using type = int32_t;
};
template<> struct _extended_signed<uint16_t>{
	using type = int32_t;
};
template<> struct _extended_signed<uint32_t>{
	using type = int64_t;
};
template<> struct _extended_signed<int8_t>{
	using type = int32_t;
};
template<> struct _extended_signed<int16_t>{
	using type = int32_t;
};
template<> struct _extended_signed<int32_t>{
	using type = int64_t;
};
template<> struct _extended_signed<int>{
	using type = int64_t;
};
}

template<typename T>
using extended_unsigned_t = typename details::_extended_unsigned<T>::type;

template<typename T>
using extended_signed_t = typename details::_extended_signed<T>::type;

}