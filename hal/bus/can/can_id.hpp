#pragma once

#include <cstdint>
#include <compare>

namespace ymd::hal{

namespace details{

template<typename T>
class CanId_t{
public:
    static constexpr CanId_t<T> ZERO = CanId_t<T>(0);

    constexpr CanId_t(const T raw):raw_(raw){;}

    constexpr auto operator<=>(const CanId_t<T>& other) const = default;

    constexpr uint16_t as_raw() const {return raw_;}
private:
    T raw_;
};

class CanRawId{
public:
    CanRawId(const uint32_t raw):raw_(raw){;}
    uint32_t as_raw() const {return raw_;}
private:
    uint32_t raw_;
};

}

using CanStdId = details::CanId_t<uint16_t>;
using CanExtId = details::CanId_t<uint16_t>;


}