#include "SdoObjDict.hpp"
#include "core/string/string.hpp"


using namespace ymd::canopen;
using namespace ymd;




Option<SubEntry> SdoObjDict::get_sub_entry(const Didx didx) {
    const auto [idx, subidx] = didx;

    switch (idx){
        case 0x1200: switch (subidx) {
            case 0x01:
                return Some(SubEntry::from_u16_rw(
                    CANOPEN_NAME("client to server cobid"), c2s_cobid.to_u16()));
            case 0x02:
                return Some(SubEntry::from_u16_rw(
                    CANOPEN_NAME("server to client cobid"), s2c_cobid.to_u16()));
            default:break;
        }break;
        case 0x1201:switch (subidx) {
            case 0x01:
                return Some(SubEntry::from_u16_rw(
                    CANOPEN_NAME("client to server cobid"), c2s_cobid.to_u16()));
            case 0x02:
                return Some(SubEntry::from_u16_rw(
                    CANOPEN_NAME("server to client cobid"), s2c_cobid.to_u16()));
            default: break;
        }break;
        case 0x1202:switch (subidx) {
            case 0x01:
                return Some(SubEntry::from_u16_rw(
                    CANOPEN_NAME("heartbeat time"), heartbeat_time));
            case 0x02:
                return Some(SubEntry::from_u16_rw(
                    CANOPEN_NAME("node guarding time"), node_guarding_time));
            default: break;
        }break;
        default: break;
    }
    return None;
}