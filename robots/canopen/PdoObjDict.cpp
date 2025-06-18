#include "PdoObjDict.hpp"

using namespace ymd;
using namespace ymd::canopen;

Option<SubEntry> PdoObjDict::get_sub_entry(const Didx didx) {
    const auto [idx, subidx] = didx;

    switch (idx) {
        case 0x1600: switch (subidx) {
            case 0x00:
                return Some(SubEntry::from_u16_rw(
                    CANOPEN_NAME("transmit PDO communication parameter"), tx_param));
            case 0x01:
                return Some(SubEntry::from_u32_rw(
                    CANOPEN_NAME("transmit PDO mapping parameter"), rx_param));
            default: break;
            }
        // case 0x1A00: switch (subidx) {
            // case 0x00:
            //     return SubEntry{CANOPEN_NAME("receive PDO communication parameter"), rxpdo_communication_parameter_, EntryAccessAuthority::RW, EntryDataType::uint16};
            // case 0x01:
            //     return SubEntry{CANOPEN_NAME("receive PDO mapping parameter"), rxpdo_mapping_parameter_, EntryAccessAuthority::RW, EntryDataType::uint32};
            // default: break;
        //     }

        default:
            return None;
    }
}