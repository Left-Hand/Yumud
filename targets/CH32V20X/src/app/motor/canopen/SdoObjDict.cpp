#include "ObjectDict.hpp"
#include "sys/string/string.hpp"


using namespace ymd::canopen;
using namespace ymd;

constexpr uint32_t mask(const OdIndex idx, const OdSubIndex subidx){
    // const int m = (uint32_t(idx) << 16) | subidx;
    // const char chars = std::bit_cast<char[4]>((uint32_t(idx) << 16) | subidx);
    const char chars[4] = {
        char(idx), char(idx >> 8), char(subidx), 0,
    };
    // const int m = (subidx << 16) | idx;
    return hash_impl(&chars[0], 4);
    // return m;
}

constexpr SubEntry make_subentry(const StringView name, auto & val, SubEntry::AccessType access_type, SubEntry::DataType data_type){
    return SubEntry{name, val, access_type, data_type};
}
std::optional<SubEntry> SdoObjectDict::find(const std::pair<const Index, const SubIndex> didx) {
    const auto [idx, subidx] = didx;

    switch (idx) {
        case 0x1200: switch (subidx) {
                case 0x01:
                    return SubEntry{NAME("client to server cobid"), c2s_cobid_, EntryAccessType::RW, EntryDataType::uint16};
                case 0x02:
                    return SubEntry{NAME("server to client cobid"), s2c_cobid_, EntryAccessType::RW, EntryDataType::uint16};
                default:goto end;
        }goto end;
        case 0x1201:switch (subidx) {
                case 0x01:
                    return SubEntry{NAME("client to server cobid"), c2s_cobid_, EntryAccessType::RW, EntryDataType::uint16};
                case 0x02:
                    return SubEntry{NAME("server to client cobid"), s2c_cobid_, EntryAccessType::RW, EntryDataType::uint16};
                default: goto end;
        }goto end;
        case 0x1202:switch (subidx) {
                case 0x01:
                    return SubEntry{NAME("heartbeat time"), heartbeat_time_, EntryAccessType::RW, EntryDataType::uint16};
                case 0x02:
                    return SubEntry{NAME("node guarding time"), node_guarding_time_, EntryAccessType::RW, EntryDataType::uint16};
                default: goto end;
        }goto end;
        default: goto end;
    }
end:
    return std::nullopt;
}


std::optional<SubEntry> NmtObjectDict::find(const std::pair<const Index, const SubIndex> didx) {
    const auto [idx, subidx] = didx;

    switch (idx) {
        case 0x1000: switch (subidx) {
                case 0x01:
                    return SubEntry{NAME("device type"), device_type_, EntryAccessType::RO, EntryDataType::uint32};

                default: goto end;
            }goto end;
        case 0x1001: switch (subidx) {
                case 0x01:
                    return SubEntry{NAME("error register"), error_register_, EntryAccessType::RO, EntryDataType::uint8};

                default: goto end;
            }goto end;
        case 0x1002: switch (subidx) {
                case 0x01:
                    return SubEntry{NAME("manufacturer status"), manufacturer_status_, EntryAccessType::RO, EntryDataType::uint32};

                default: goto end;
            }goto end;
        case 0x1003: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("number of errors"), num_errors_, EntryAccessType::RO, EntryDataType::uint8};
                case 0x01:
                    return SubEntry{NAME("error code 1"), error_code_1_, EntryAccessType::RO, EntryDataType::uint32};
                case 0x02:
                    return SubEntry{NAME("error code 2"), error_code_2_, EntryAccessType::RO, EntryDataType::uint32};

                default: goto end;
            }goto end;
        // case 0x1008: switch (subidx) {
        //         case 0x01:
        //             return SubEntry{NAME("device name"), device_name_, EntryAccessType::RO, EntryDataType::string};
        //         // 添加其他 subidx 的 case
        //         goto end;
        //     }
        case 0x2000: switch (subidx) {
                case 0x01:
                    return SubEntry{NAME("manufacturer specific 1"), manufacturer_specific_1_, EntryAccessType::RW, EntryDataType::uint32};
                case 0x02:
                    return SubEntry{NAME("manufacturer specific 2"), manufacturer_specific_2_, EntryAccessType::RW, EntryDataType::uint32};

                default: goto end;
            }goto end;
        default: goto end;
    }
end:
    return std::nullopt;
}


std::optional<SubEntry> Cia402ObjectDict::find(const std::pair<const Index, const SubIndex> didx) {
    const auto [idx, subidx] = didx;

    switch (idx) {
        case 0x6000: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("control word"), control_word_, EntryAccessType::RW, EntryDataType::uint16};
                case 0x01:
                    return SubEntry{NAME("status word"), status_word_, EntryAccessType::RO, EntryDataType::uint16};
                default: goto end;
            }break;
        case 0x6060: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("modes of operation"), modes_of_operation_, EntryAccessType::RW, EntryDataType::int8};
                case 0x01:
                    return SubEntry{NAME("modes of operation display"), modes_of_operation_display_, EntryAccessType::RO, EntryDataType::int8};
                default: goto end;
            }break;
        case 0x6061: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("position actual value"), position_actual_value_, EntryAccessType::RO, EntryDataType::int32};
                default: goto end;
            }break;
        case 0x6062: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("velocity actual value"), velocity_actual_value_, EntryAccessType::RO, EntryDataType::int32};
                default: goto end;
            }break;
        case 0x6063: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("torque actual value"), torque_actual_value_, EntryAccessType::RO, EntryDataType::int16};
                default: goto end;
            }break;
        case 0x6064: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("position demand value"), position_demand_value_, EntryAccessType::RW, EntryDataType::int32};
                default: goto end;
            }break;
        case 0x6065: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("velocity demand value"), velocity_demand_value_, EntryAccessType::RW, EntryDataType::int32};
                default: goto end;
            }break;
        case 0x6066: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("torque demand value"), torque_demand_value_, EntryAccessType::RW, EntryDataType::int16};
                default: goto end;
            }break;
        case 0x607A: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("target position"), target_position_, EntryAccessType::RW, EntryDataType::int32};
                default: goto end;
            }break;
        case 0x607B: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("target velocity"), target_velocity_, EntryAccessType::RW, EntryDataType::int32};
                default: goto end;
            }break;
        case 0x607D: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("target torque"), target_torque_, EntryAccessType::RW, EntryDataType::int16};
                default: goto end;
            }break;
        case 0x6081: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("max profile velocity"), max_profile_velocity_, EntryAccessType::RW, EntryDataType::int32};
                default: goto end;
            }break;
        case 0x6083: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("max motor current"), max_motor_current_, EntryAccessType::RW, EntryDataType::int16};
                default: goto end;
            }break;
        case 0x6085: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("max profile acceleration"), max_profile_acceleration_, EntryAccessType::RW, EntryDataType::int32};
                default: goto end;
            }break;
        case 0x6087: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("max profile deceleration"), max_profile_deceleration_, EntryAccessType::RW, EntryDataType::int32};
                default: goto end;
            }break;
        case 0x6098: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("software position limit"), software_position_limit_, EntryAccessType::RW, EntryDataType::int32};
                default: goto end;
            }break;
        case 0x609A: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("software velocity limit"), software_velocity_limit_, EntryAccessType::RW, EntryDataType::int32};
                default: goto end;
            }break;
        case 0x609C: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("software torque limit"), software_torque_limit_, EntryAccessType::RW, EntryDataType::int16};
                default: goto end;
            }break;
        default:
            goto end;
    }
end:
    return std::nullopt;
}