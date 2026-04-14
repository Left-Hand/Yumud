#include "bxcan_frame.hpp"
#include "bxcan_payload.hpp"
#include "core/stream/ostream.hpp"
#include "core/string/view/string_view.hpp"

namespace ymd{

OutputStream & operator<<(OutputStream & os, const hal::ClassicCanFrame & frame){
    const auto guard = os.create_guard();

    os << '<'
        << ((frame.is_standard()) ? StringView("Std") : StringView("Ext"))
        << ((frame.is_remote()) ? StringView("Remote") : StringView("Data"))
        << StringView("> ");

    os << StringView("id=") << std::hex << std::showbase << frame.id_u32();

    if(not frame.is_remote()){
        os << StringView(" |") << frame.payload();
    }

    return os;
}

OutputStream & operator<<(OutputStream & os, const hal::ClassicCanPayload & payload){
    const auto guard = os.create_guard();

    os << '[' << std::dec << payload.length() << StringView("]{")
        << std::showbase << std::hex << payload.bytes() << '}';

    return os;
}

}