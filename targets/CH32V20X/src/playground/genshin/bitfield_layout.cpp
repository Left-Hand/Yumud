#include <cstdint>
#include <bit>
#include "core/tmp/bits/width.hpp"


namespace ymd{
struct ExampleReg{
    uint32_t CPHA:1;
    uint32_t CPOL:1;
    uint32_t MSTR:1;
    uint32_t BR:3;
    uint32_t SPE:1;
    uint32_t LSB:1;

    uint32_t SSI:1;
    uint32_t SSM:1;
    uint32_t RXONLY:1;
    uint32_t DFF:1;
    uint32_t CRCNEXT:1;
    uint32_t CRCEN:1;
    uint32_t BIDIOE:1;
    uint32_t BIDIMODE:1;

    uint32_t __RESV__:16;
};


template<typename T>
struct bitfield_reflecter{
private:
    using D = tmp::size_to_uint_t<sizeof(T)>;
    static constexpr T ZERO = std::bit_cast<T>(static_cast<D>(0));

    template<auto p>
    struct ReadMemPtr {
        static_assert(p!=p,"not a member pointer");
    };
public:
    template<auto p>
    struct mask{
        static constexpr D value = []{
            auto reg = ZERO;
            reg.*p = tmp::int_to_full_v<D>;
            return std::bit_cast<D>(reg);
        }();
    };
    // template<auto p>
    // struct _offset{

    // };

    // template<auto p>
    // static constexpr size_t offset_v = _offset<P>::value;
};

#define DEF_REG_FIELD_MASK(reg_name, field_name)\
[] -> uint32_t{\
    constexpr auto k_field_reg = std::bit_cast<reg_name>(uint32_t(0));\
    auto field_reg = k_field_reg;\
    field_reg.field_name = 1u;\
    return std::bit_cast<uint32_t>(field_reg);\
}();\

static constexpr auto mask = DEF_REG_FIELD_MASK(ExampleReg, BIDIMODE);
static_assert(mask == 0x008000);

// constexpr uint32_t m2 = bitfield_reflecter<ExampleReg>::template mask<&ExampleReg::BIDIMODE>::value;


}