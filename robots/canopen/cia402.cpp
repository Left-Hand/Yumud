#include "cia402.hpp"

using namespace ymd::canopen;

std::optional<SubEntry> Cia402ObjectDict::find(const Didx didx) {

    const auto [idx, subidx] = didx;

    switch (idx) {
        case 0x6000: switch (subidx) {
                case 0x00:
                    return SubEntry{CANOPEN_NAME("control word"), control_word_, EntryAccessAuthority::RW, EntryDataType::uint16};
                case 0x01:
                    return SubEntry{CANOPEN_NAME("status word"), status_word_, EntryAccessAuthority::RO, EntryDataType::uint16};
                default: break;
            }break;
        case 0x6060: switch (subidx) {
                case 0x00:
                    return SubEntry{CANOPEN_NAME("modes of operation"), modes_of_operation_, EntryAccessAuthority::RW, EntryDataType::int8};
                case 0x01:
                    return SubEntry{CANOPEN_NAME("modes of operation display"), modes_of_operation_display_, EntryAccessAuthority::RO, EntryDataType::int8};
                default: break;
            }break;
        case 0x6061: switch (subidx) {
                case 0x00:
                    return SubEntry{CANOPEN_NAME("position actual value"), position_actual_value_, EntryAccessAuthority::RO, EntryDataType::int32};
                default: break;
            }break;
        case 0x6062: switch (subidx) {
                case 0x00:
                    return SubEntry{CANOPEN_NAME("velocity actual value"), velocity_actual_value_, EntryAccessAuthority::RO, EntryDataType::int32};
                default: break;
            }break;
        case 0x6063: switch (subidx) {
                case 0x00:
                    return SubEntry{CANOPEN_NAME("torque actual value"), torque_actual_value_, EntryAccessAuthority::RO, EntryDataType::int16};
                default: break;
            }break;
        case 0x6064: switch (subidx) {
                case 0x00:
                    return SubEntry{CANOPEN_NAME("position demand value"), position_demand_value_, EntryAccessAuthority::RW, EntryDataType::int32};
                default: break;
            }break;
        case 0x6065: switch (subidx) {
                case 0x00:
                    return SubEntry{CANOPEN_NAME("velocity demand value"), velocity_demand_value_, EntryAccessAuthority::RW, EntryDataType::int32};
                default: break;
            }break;
        case 0x6066: switch (subidx) {
                case 0x00:
                    return SubEntry{CANOPEN_NAME("torque demand value"), torque_demand_value_, EntryAccessAuthority::RW, EntryDataType::int16};
                default: break;
            }break;
        case 0x607A: switch (subidx) {
                case 0x00:
                    return SubEntry{CANOPEN_NAME("target position"), target_position_, EntryAccessAuthority::RW, EntryDataType::int32};
                default: break;
            }break;
        case 0x607B: switch (subidx) {
                case 0x00:
                    return SubEntry{CANOPEN_NAME("target velocity"), target_velocity_, EntryAccessAuthority::RW, EntryDataType::int32};
                default: break;
            }break;
        case 0x607D: switch (subidx) {
                case 0x00:
                    return SubEntry{CANOPEN_NAME("target torque"), target_torque_, EntryAccessAuthority::RW, EntryDataType::int16};
                default: break;
            }break;
        case 0x6081: switch (subidx) {
                case 0x00:
                    return SubEntry{CANOPEN_NAME("max profile velocity"), max_profile_velocity_, EntryAccessAuthority::RW, EntryDataType::int32};
                default: break;
            }break;
        case 0x6083: switch (subidx) {
                case 0x00:
                    return SubEntry{CANOPEN_NAME("max motor current"), max_motor_current_, EntryAccessAuthority::RW, EntryDataType::int16};
                default: break;
            }break;
        case 0x6085: switch (subidx) {
                case 0x00:
                    return SubEntry{CANOPEN_NAME("max profile acceleration"), max_profile_acceleration_, EntryAccessAuthority::RW, EntryDataType::int32};
                default: break;
            }break;
        case 0x6087: switch (subidx) {
                case 0x00:
                    return SubEntry{CANOPEN_NAME("max profile deceleration"), max_profile_deceleration_, EntryAccessAuthority::RW, EntryDataType::int32};
                default: break;
            }break;
        case 0x6098: switch (subidx) {
                case 0x00:
                    return SubEntry{CANOPEN_NAME("software position limit"), software_position_limit_, EntryAccessAuthority::RW, EntryDataType::int32};
                default: break;
            }break;
        case 0x609A: switch (subidx) {
                case 0x00:
                    return SubEntry{CANOPEN_NAME("software velocity limit"), software_velocity_limit_, EntryAccessAuthority::RW, EntryDataType::int32};
                default: break;
            }break;
        case 0x609C: switch (subidx) {
                case 0x00:
                    return SubEntry{CANOPEN_NAME("software torque limit"), software_torque_limit_, EntryAccessAuthority::RW, EntryDataType::int16};
                default: break;
            }break;
        default:
            return Cia301ObjectDict::find(didx);
    }
    return std::nullopt;
}
