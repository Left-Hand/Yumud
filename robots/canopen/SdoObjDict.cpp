#include "SdoObjDict.hpp"
#include "core/string/string.hpp"


using namespace ymd::canopen;
using namespace ymd;




std::optional<SubEntry> SdoObjDict::find(const Didx didx) {
    const auto [idx, subidx] = didx;

    switch (idx){
        case 0x1200: switch (subidx) {
            case 0x01:
                return SubEntry{CANOPEN_NAME("client to server cobid"), c2s_cobid.to_u16(), EntryAccessAuthority::RW, EntryDataType::uint16};
            case 0x02:
                return SubEntry{CANOPEN_NAME("server to client cobid"), s2c_cobid.to_u16(), EntryAccessAuthority::RW, EntryDataType::uint16};
            default:break;
        }break;
        case 0x1201:switch (subidx) {
            case 0x01:
                return SubEntry{CANOPEN_NAME("client to server cobid"), c2s_cobid.to_u16(), EntryAccessAuthority::RW, EntryDataType::uint16};
            case 0x02:
                return SubEntry{CANOPEN_NAME("server to client cobid"), s2c_cobid.to_u16(), EntryAccessAuthority::RW, EntryDataType::uint16};
            default: break;
        }break;
        case 0x1202:switch (subidx) {
            case 0x01:
                return SubEntry{CANOPEN_NAME("heartbeat time"), heartbeat_time, EntryAccessAuthority::RW, EntryDataType::uint16};
            case 0x02:
                return SubEntry{CANOPEN_NAME("node guarding time"), node_guarding_time, EntryAccessAuthority::RW, EntryDataType::uint16};
            default: break;
        }break;
        default: break;
    }
    return std::nullopt;
}