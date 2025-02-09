#include "SdoObjDict.hpp"
#include "sys/string/string.hpp"


using namespace ymd::canopen;
using namespace ymd;




std::optional<SubEntry> SdoObjDict::find(const Didx didx) {
    const auto [idx, subidx] = didx;

    switch (idx) {
        case 0x1200: switch (subidx) {
                case 0x01:
                    return SubEntry{NAME("client to server cobid"), c2s_cobid, EntryAccessType::RW, EntryDataType::uint16};
                case 0x02:
                    return SubEntry{NAME("server to client cobid"), s2c_cobid, EntryAccessType::RW, EntryDataType::uint16};
                default:break;
        }break;
        case 0x1201:switch (subidx) {
                case 0x01:
                    return SubEntry{NAME("client to server cobid"), c2s_cobid, EntryAccessType::RW, EntryDataType::uint16};
                case 0x02:
                    return SubEntry{NAME("server to client cobid"), s2c_cobid, EntryAccessType::RW, EntryDataType::uint16};
                default: break;
        }break;
        case 0x1202:switch (subidx) {
                case 0x01:
                    return SubEntry{NAME("heartbeat time"), heartbeat_time, EntryAccessType::RW, EntryDataType::uint16};
                case 0x02:
                    return SubEntry{NAME("node guarding time"), node_guarding_time, EntryAccessType::RW, EntryDataType::uint16};
                default: break;
        }break;
        default: break;
    }
    return std::nullopt;
}