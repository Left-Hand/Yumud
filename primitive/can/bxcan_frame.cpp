#include "bxcan_frame.hpp"
#include "bxcan_payload.hpp"
#include "core/stream/ostream.hpp"
#include "core/string/view/string_view.hpp"

namespace {
static constexpr void fmthex(
    char * p_str, 
    std::size_t len,
    uint32_t unsigned_val
) {
    for(size_t offset = len - 1; offset != static_cast<size_t>(-1); --offset){
        const uint8_t digit = unsigned_val & 0b1111;  // Get lowest 4 bits (hex digit)
        p_str[offset] = digit > 9 ? (digit - 10 + 'A') : (digit + '0');
        unsigned_val >>= 4;                     // Move to next hex digit
    }
}

}
namespace ymd{

struct [[nodiscard]] PrettyIdConverter final{
    hal::CanIdentifier id;

    friend OutputStream & operator <<(OutputStream & os, const PrettyIdConverter self){
        const uint32_t id_u32 = self.id.id_u32();

        std::array<char, 12> buf;
        char * pbuf = buf.data();

        pbuf[0] = '0';
        pbuf[1] = 'x';
        pbuf += 2;

        const size_t num_id_chars = self.id.is_extended() ? 8 : 3;
        fmthex(pbuf, num_id_chars, id_u32);
        pbuf += num_id_chars;
        
        return os << (StringView(buf.data(), pbuf));
    }
};

OutputStream & operator<<(OutputStream & os, const hal::ClassicCanFrame & frame){
    const auto guard = os.create_guard();

    os << '<'
        << ((frame.is_standard()) ? ("Std") : ("Ext"))
        << ((frame.is_remote()) ? ("Rmt") : ("Dat"))
        << ("> ");

    os << ("id=") << PrettyIdConverter{.id = frame.identifier()};

    if(not frame.is_remote()){
        os << (" |") << frame.payload();
    }

    return os;
}

OutputStream & operator<<(OutputStream & os, const hal::ClassicCanPayload & payload){
    const auto guard = os.create_guard();

    os << '[' << std::dec << payload.length() << "]{"
        << std::showbase << std::hex << payload.bytes() << '}';

    return os;
}

}