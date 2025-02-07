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
    // const auto idx = mask(didx.first, didx.second);

    // switch (idx) {
    

    //     // case mask(0x1000, 0x01):
    //     //     return make_subentry("Device Type", device_type_, SubEntry::AccessType::RO, SubEntry::DataType::uint32);

    //     // // 错误寄存器 (Error Register)
    //     // case mask(0x1001, 0x01):
    //     //     return make_subentry("Error Register", error_register_, SubEntry::AccessType::RO, SubEntry::DataType::uint8);

    //     // // 制造商状态寄存器 (Manufacturer Status Register)
    //     // case mask(0x1002, 0x01):
    //     //     return make_subentry("Manufacturer Status Register", manufacturer_status_, SubEntry::AccessType::RO, SubEntry::DataType::uint32);

    //     // // 预定义错误字段 (Pre-defined Error Field)
    //     // case mask(0x1003, 0x00):
    //     //     return make_subentry("Number of Errors", num_errors_, SubEntry::AccessType::RO, SubEntry::DataType::uint8);
    //     // case mask(0x1003, 0x01):
    //     //     return make_subentry("Error Code 1", error_code_1_, SubEntry::AccessType::RO, SubEntry::DataType::uint32);
    //     // case mask(0x1003, 0x02):
    //     //     return make_subentry("Error Code 2", error_code_2_, SubEntry::AccessType::RO, SubEntry::DataType::uint32);

    //     // 心跳生产者时间 (Heartbeat Producer Time)
    //     case mask(0x1017, 0x01):
    //         return make_subentry("Heartbeat Time", heartbeat_time_, SubEntry::AccessType::RW, SubEntry::DataType::uint16);

    //     // SDO服务器参数 (SDO Server Parameters)
    //     case mask(0x1200, 0x01):
    //         return make_subentry("Client to Server COB-ID", c2s_cobid_, SubEntry::AccessType::RW, SubEntry::DataType::uint16);
    //     case mask(0x1200, 0x02):
    //         return make_subentry("Server to Client COB-ID", s2c_cobid_, SubEntry::AccessType::RW, SubEntry::DataType::uint16);
    //     case mask(0x1201, 0x01):
    //         return make_subentry("Client to Server COB-ID", c2s_cobid_, SubEntry::AccessType::RW, SubEntry::DataType::uint16);
    //     case mask(0x1201, 0x02):
    //         return make_subentry("Server to Client COB-ID", s2c_cobid_, SubEntry::AccessType::RW, SubEntry::DataType::uint16);
    //     case mask(0x1202, 0x01):
    //         return make_subentry("Heartbeat Time", heartbeat_time_, SubEntry::AccessType::RW, SubEntry::DataType::uint16);
    //     case mask(0x1202, 0x02):
    //         return make_subentry("Node Guarding Time", node_guarding_time_, SubEntry::AccessType::RW, SubEntry::DataType::uint16);
    //     case mask(0x1203, 0x01):
    //         return make_subentry("Sync Period", sync_period_, SubEntry::AccessType::RW, SubEntry::DataType::uint32);
    //     case mask(0x1203, 0x02):
    //         return make_subentry("Sync Window Length", sync_window_length_, SubEntry::AccessType::RW, SubEntry::DataType::uint32);
    //     case mask(0x1204, 0x01):
    //         return make_subentry("Emergency Consumer COB-ID", emergency_consumer_cobid_, SubEntry::AccessType::RW, SubEntry::DataType::uint32);
    //     case mask(0x1204, 0x02):
    //         return make_subentry("Emergency Producer COB-ID", emergency_producer_cobid_, SubEntry::AccessType::RW, SubEntry::DataType::uint32);

    //     // // 设备名称 (Device Name)
    //     // case mask(0x1008, 0x01):
    //     //     return make_subentry("Device Name", device_name_, SubEntry::AccessType::RO, SubEntry::DataType::string);

    //     // // 制造商特定对象 (Manufacturer Specific Objects)
    //     // case mask(0x2000, 0x01):
    //     //     return make_subentry("Manufacturer Specific 1", manufacturer_specific_1_, SubEntry::AccessType::RW, SubEntry::DataType::uint32);
    //     // case mask(0x2000, 0x02):
    //     //     return make_subentry("Manufacturer Specific 2", manufacturer_specific_2_, SubEntry::AccessType::RW, SubEntry::DataType::uint32);
    // }

    const auto [idx, subidx] = didx;

    switch (idx) {
        case 0x1200: switch (subidx) {
                case 0x01:
                    return SubEntry{NAME("client to server cobid"), c2s_cobid_, EntryAccessType::RW, EntryDataType::uint16};
                case 0x02:
                    return SubEntry{NAME("server to client cobid"), s2c_cobid_, EntryAccessType::RW, EntryDataType::uint16};
            break;
        }
        case 0x1201:switch (subidx) {
                case 0x01:
                    return SubEntry{NAME("client to server cobid"), c2s_cobid_, EntryAccessType::RW, EntryDataType::uint16};
                case 0x02:
                    return SubEntry{NAME("server to client cobid"), s2c_cobid_, EntryAccessType::RW, EntryDataType::uint16};
            break;
        }
        case 0x1202:switch (subidx) {
                case 0x01:
                    return SubEntry{NAME("heartbeat time"), heartbeat_time_, EntryAccessType::RW, EntryDataType::uint16};
                case 0x02:
                    return SubEntry{NAME("node guarding time"), node_guarding_time_, EntryAccessType::RW, EntryDataType::uint16};
            break;
        }
        default:
            break;
    }
    return std::nullopt;
}