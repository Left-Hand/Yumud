#include "bxcan_frame.hpp"
#include "bxcan_payload.hpp"
#include "core/stream/ostream.hpp"
#include "core/string/view/string_view.hpp"
#include "core/string/conv/fmtnum/fmtnum.hpp"

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
        str::_fmtnum_u32_r16(pbuf, id_u32, num_id_chars);
        pbuf += num_id_chars;
        
        return os << (StringView(buf.data(), pbuf));
    }
};

OutputStream & operator<<(OutputStream & os, const hal::ClassicCanFrame & frame){
    const auto guard = os.create_guard();

    os << '<'
        << ((frame.is_standard()) ? StringView("Std") : StringView("Ext"))
        << ((frame.is_remote()) ? StringView("Rmt") : StringView("Dat"))
        << StringView("> ");

    os << StringView("id=") << PrettyIdConverter{.id = frame.identifier()};

    if(not frame.is_remote()){
        os << StringView(" |") << frame.payload();
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