#include "cia402.hpp"

using namespace ymd::canopen;

std::optional<SubEntry> Cia402ObjectDict::find(const std::pair<const Index, const SubIndex> didx) {

    const auto [idx, subidx] = didx;

    switch (idx) {
        case 0x6000: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("control word"), control_word_, EntryAccessType::RW, EntryDataType::uint16};
                case 0x01:
                    return SubEntry{NAME("status word"), status_word_, EntryAccessType::RO, EntryDataType::uint16};
                default: break;
            }break;
        case 0x6060: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("modes of operation"), modes_of_operation_, EntryAccessType::RW, EntryDataType::int8};
                case 0x01:
                    return SubEntry{NAME("modes of operation display"), modes_of_operation_display_, EntryAccessType::RO, EntryDataType::int8};
                default: break;
            }break;
        case 0x6061: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("position actual value"), position_actual_value_, EntryAccessType::RO, EntryDataType::int32};
                default: break;
            }break;
        case 0x6062: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("velocity actual value"), velocity_actual_value_, EntryAccessType::RO, EntryDataType::int32};
                default: break;
            }break;
        case 0x6063: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("torque actual value"), torque_actual_value_, EntryAccessType::RO, EntryDataType::int16};
                default: break;
            }break;
        case 0x6064: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("position demand value"), position_demand_value_, EntryAccessType::RW, EntryDataType::int32};
                default: break;
            }break;
        case 0x6065: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("velocity demand value"), velocity_demand_value_, EntryAccessType::RW, EntryDataType::int32};
                default: break;
            }break;
        case 0x6066: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("torque demand value"), torque_demand_value_, EntryAccessType::RW, EntryDataType::int16};
                default: break;
            }break;
        case 0x607A: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("target position"), target_position_, EntryAccessType::RW, EntryDataType::int32};
                default: break;
            }break;
        case 0x607B: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("target velocity"), target_velocity_, EntryAccessType::RW, EntryDataType::int32};
                default: break;
            }break;
        case 0x607D: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("target torque"), target_torque_, EntryAccessType::RW, EntryDataType::int16};
                default: break;
            }break;
        case 0x6081: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("max profile velocity"), max_profile_velocity_, EntryAccessType::RW, EntryDataType::int32};
                default: break;
            }break;
        case 0x6083: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("max motor current"), max_motor_current_, EntryAccessType::RW, EntryDataType::int16};
                default: break;
            }break;
        case 0x6085: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("max profile acceleration"), max_profile_acceleration_, EntryAccessType::RW, EntryDataType::int32};
                default: break;
            }break;
        case 0x6087: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("max profile deceleration"), max_profile_deceleration_, EntryAccessType::RW, EntryDataType::int32};
                default: break;
            }break;
        case 0x6098: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("software position limit"), software_position_limit_, EntryAccessType::RW, EntryDataType::int32};
                default: break;
            }break;
        case 0x609A: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("software velocity limit"), software_velocity_limit_, EntryAccessType::RW, EntryDataType::int32};
                default: break;
            }break;
        case 0x609C: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("software torque limit"), software_torque_limit_, EntryAccessType::RW, EntryDataType::int16};
                default: break;
            }break;
        default:
            return Cia301ObjectDict::find(didx);
    }
    return std::nullopt;
}
