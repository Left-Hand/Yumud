#pragma once

#include "algebra/regions/range2.hpp"

namespace ymd{


class Memory;

struct [[nodiscard]] AddressDiff final{
    [[nodiscard]] constexpr explicit AddressDiff(const uint32_t diff):count_(diff){;}
    [[nodiscard]] constexpr uint32_t to_u32() const {return count_;}
    [[nodiscard]] constexpr auto operator<=>(const AddressDiff &) const = default;
private:
    uint32_t count_;
};

using Capacity = AddressDiff;


struct [[nodiscard]] Address final{
    [[nodiscard]] constexpr explicit Address(const uint32_t addr):count_(addr){;}
    [[nodiscard]] constexpr uint32_t to_u32() const {return count_;}

    [[nodiscard]] constexpr auto operator<=>(const Address &) const = default;
    [[nodiscard]] constexpr AddressDiff operator - (const Address &rhs) const {return AddressDiff(count_ - rhs.count_);}
    [[nodiscard]] constexpr Address operator - (const AddressDiff &rhs) const {return Address(count_ - rhs.to_u32());}
    [[nodiscard]] constexpr Address operator + (const AddressDiff &rhs) const {return Address(count_ + rhs.to_u32());}
private:
    uint32_t count_;
};


consteval Address operator"" _addr(unsigned long long  x){
    return Address(static_cast<uint32_t>(x));
}

inline OutputStream &operator << (OutputStream &os, const Address &addr){
    const auto guard = os.create_guard();
    os << std::hex << std::showbase << addr.to_u32();
    return os;
}


struct [[nodiscard]] AddressRange final{
    Address from;
    Address to;

    constexpr explicit AddressRange(const Address _from, const Address _to):
        from(_from),to(_to){;}

    constexpr explicit AddressRange(const Address _addr, const AddressDiff _AddressDiff):
        from(_addr),to(_addr + _AddressDiff){;}

    constexpr AddressDiff capacity() const{ return to - from; }
};


inline OutputStream &operator<<(OutputStream &os, const AddressRange &range){
    const auto guard = os.create_guard();
    os << os.brackets<'['>() << range.from << ":" << range.to << os.brackets<')'>();
    return os;
}




}