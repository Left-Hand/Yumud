#include "ObjectDict.hpp"
#include "sys/string/string.hpp"


using namespace ymd::canopen;
using namespace ymd;


template<typename T>
struct reg_decay{
    using type = std::conditional_t<
        std::is_base_of_v<__RegBase, T>,
        typename T::value_type, T>;
};


template<typename T>
using reg_decay_t = typename reg_decay<T>::type;


template<typename T>
constexpr SubEntry make_subentry_impl(
        const StringView name, 
        T & val, 
        EntryAccessType access_type = std::is_const_v<T> ? EntryAccessType::RO : EntryAccessType::RW, 
        EntryDataType data_type = EntryDataType::from<reg_decay_t<T>>()){
    return SubEntry{name, val, access_type, data_type};
}


#define make_subentry(val) make_subentry_impl(NAME(#val), val);
#define make_ro_subentry(val) make_subentry_impl(NAME(#val), val, EntryAccessType::RO);
#define make_subentry_spec(val, access_type, data_type) make_subentry_impl(NAME(#val), val, access_type, data_type);


std::optional<SubEntry> SdoObjectDict::find(const std::pair<const Index, const SubIndex> didx) {
    const auto [idx, subidx] = didx;

    switch (idx) {
        case 0x1200: switch (subidx) {
                case 0x01:
                    return SubEntry{NAME("client to server cobid"), c2s_cobid_, EntryAccessType::RW, EntryDataType::uint16};
                case 0x02:
                    return SubEntry{NAME("server to client cobid"), s2c_cobid_, EntryAccessType::RW, EntryDataType::uint16};
                default:break;
        }break;
        case 0x1201:switch (subidx) {
                case 0x01:
                    return SubEntry{NAME("client to server cobid"), c2s_cobid_, EntryAccessType::RW, EntryDataType::uint16};
                case 0x02:
                    return SubEntry{NAME("server to client cobid"), s2c_cobid_, EntryAccessType::RW, EntryDataType::uint16};
                default: break;
        }break;
        case 0x1202:switch (subidx) {
                case 0x01:
                    return SubEntry{NAME("heartbeat time"), heartbeat_time_, EntryAccessType::RW, EntryDataType::uint16};
                case 0x02:
                    return SubEntry{NAME("node guarding time"), node_guarding_time_, EntryAccessType::RW, EntryDataType::uint16};
                default: break;
        }break;
        default: break;
    }
    return std::nullopt;
}

std::optional<SubEntry> Cia301ObjectDict::find(const std::pair<const Index, const SubIndex> didx) {
    const auto [idx, subidx] = didx;

    switch (idx) {
            
        case 0x1000: switch (subidx) {
            case 0x0:
                return make_ro_subentry(control_word_reg);
        }break;

        case 0x1001: switch (subidx) {
            case 0x0:
                return make_ro_subentry(error_reg);
        }break;

        default: break;
    }
    return std::nullopt;
}


std::optional<SubEntry> Cia402ObjectDict::find(const std::pair<const Index, const SubIndex> didx) {

    {
        auto res = Cia301ObjectDict::find(didx);
        if(res.has_value()) return res;
    }
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
            break;
    }
    return std::nullopt;
}



class PdoObjectDict:public StaticObjectDictBase{
protected:
    struct PdoConfig{
        uint16_t tx_param_;
        uint32_t tx_mapping_;
        uint16_t rx_param_;
        uint32_t rx_mapping;
    };

    PdoConfig pdo_config_;
public:
    PdoObjectDict() = default;

    std::optional<SubEntry> find(const std::pair<const Index, const SubIndex> didx) final override;
};
    


std::optional<SubEntry> PdoObjectDict::find(const std::pair<const Index, const SubIndex> didx) {
    const auto [idx, subidx] = didx;

    switch (idx) {
        case 0x1600: switch (subidx) {
                case 0x00:
                    return SubEntry{NAME("transmit PDO communication parameter"), pdo_config_.tx_param_, EntryAccessType::RW, EntryDataType::uint16};
                case 0x01:
                    return SubEntry{NAME("transmit PDO mapping parameter"), pdo_config_.rx_param_, EntryAccessType::RW, EntryDataType::uint32};
                default: break;
            }
        // case 0x1A00: switch (subidx) {
        //         case 0x00:
        //             return SubEntry{NAME("receive PDO communication parameter"), rxpdo_communication_parameter_, EntryAccessType::RW, EntryDataType::uint16};
        //         case 0x01:
        //             return SubEntry{NAME("receive PDO mapping parameter"), rxpdo_mapping_parameter_, EntryAccessType::RW, EntryDataType::uint32};
        //         default: break;
        //     }

        default:
            break;
    }
    return std::nullopt;
}