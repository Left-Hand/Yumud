#include "sr2631z3_msgs.hpp"
#include "sr2631z3_reflect.hpp"

using namespace ymd;
using namespace ymd::drivers::sr2631z3;
using namespace msgs;

namespace{


template<typename T>
struct Uninitializer{

};

static_assert(parse_hex_str("5b").unwrap() == 0x5b);
static_assert(parse_dec_str("023656").unwrap() == 23656);
static_assert(std::get<0>(parse_floating_num("2240.61563").unwrap()) == 2240);
static_assert(std::get<1>(parse_floating_num("2240.61563").unwrap()) == 61563);
static_assert(parse_floating_num("2240.").unwrap_err() == DeMsgErrorKind::NoFrac);
static_assert(parse_floating_num(".2240").unwrap_err() == DeMsgErrorKind::NoDigit);
static_assert(parse_floating_num("22").unwrap_err() == DeMsgErrorKind::NoDot);

[[maybe_unused]] static void test_parts(){
    constexpr StringView line = "$BDGGA,023656.00,2240.61563,N,11359.86512,E,1,23,0.7,96.53,M,-3.52,M,,*5B";
    // constexpr StringView line = "$BDGGA,023656.00,2240.61563,N,11359.86512,E,1,23,0.7,96.53,M,-3.52,M,,*5A";
    constexpr auto parts = LineParts::try_from_str(line).unwrap();

    static_assert(parts.sys_id.str.hash() == "BD"_ha);
    static_assert(parts.msg_kind.str.hash() == "GGA"_ha);
    static_assert(parts.context_str.hash() == "023656.00,2240.61563,N,11359.86512,E,1,23,0.7,96.53,M,-3.52,M,,"_ha);
}


#if 0
[[maybe_unused]] static void test_gga(){
    // constexpr StringView line = "023656.00";
    constexpr StringView line = "023656.00,2240.61563,N,11359.86512,E";
    // constexpr StringView line = "023656.00,2240.61563,N,11359.86512";
    // constexpr StringView line = "023656.00,2240.61563,N,11359.86512";
    // constexpr StringView line = "$BDGGA,023656.00,2240.61563,N,11359.86512,E,1,23,0.7,96.53,M,-3.52,M,,*5A";


    // constexpr auto err = GGA::try_from_str(line).unwrap_err();
    // static_assert(int(err) == 0);
    // constexpr auto either_msg = GGA::try_from_str(line).unwrap();
    constexpr auto either_msg = try_deser_msg<msgs::GGA>(line).unwrap();

    static_assert(either_msg.utc_time.is_some());
    static_assert(either_msg.utc_time.unwrap().digit == 23656);

    static_assert(either_msg.ulat.unwrap().is_north == true);

    static_assert(either_msg.lon.is_some());
    static_assert(either_msg.lon.unwrap().digit == 11359);
    static_assert(either_msg.lon.unwrap().frac == 86512);

    static_assert(either_msg.ulon.unwrap().is_eastern == true);
}

#endif
}