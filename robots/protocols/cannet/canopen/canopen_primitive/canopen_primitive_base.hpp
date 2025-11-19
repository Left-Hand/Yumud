#pragma once

#include "canopen_funccode.hpp"

namespace ymd::canopen::primitive{

using Msg = hal::CanClassicMsg;

class SubEntry;


struct [[nodiscard]] NodeId{

    //  1：NodeID由7bit组成，其中0是保留ID，代表一个未知的节点。
    // 2：Node ID取值为1-127，包含1-127，其中126，127是保留ID。
    // 3：NodeID分为SourceNodeID和DestinationNodeID。
    // 4：SourceNodeID表示节点自身的ID。
    // 5：DestinationNodeID表示对方的节点ID。
    // 6：只有Service帧才会有DestinationNodeID，需要应答。

    using Self = NodeId;

    uint8_t bits;

    static constexpr NodeId from_u7(const uint8_t bits){
        return NodeId{static_cast<uint8_t>(bits & 0b1111111)};
    }

    static constexpr NodeId from_bits(const uint8_t bits){
        return from_u7(bits);
    }

    [[nodiscard]] constexpr uint8_t as_u7() const{
        return bits;
    }

    [[nodiscard]] constexpr bool is_boardcast() const {
        return bits == 0;
    }

    [[nodiscard]] constexpr bool test(const NodeId & other){
        return other.is_boardcast() || bits == other.bits;
    }
};



struct [[nodiscard]] CobId{

    static constexpr CobId from_parts(
        const NodeId _nodeid, 
        const FunctionCode _fcode
    ){
        const uint16_t bits = static_cast<uint16_t>(
            _nodeid.as_u7() | 
            static_cast<uint16_t>(_fcode.as_u4() << 7));

        return from_bits(bits);
    }

    constexpr hal::CanStdId to_stdid() const {
        return hal::CanStdId(nodeid_ | fcode_ << 7);
    }

    constexpr FunctionCode func_code() const {
        return FunctionCode::from_bits(static_cast<uint8_t>(fcode_));
    }

    static constexpr CobId from_bits(const uint16_t bits){
        return CobId(bits);
    }

    [[nodiscard]] constexpr uint16_t as_bits() const {
        return std::bit_cast<uint16_t>(*this);
    }

    static constexpr CobId from_stdid(const hal::CanStdId id){
        return CobId(id.to_u11());
    }

    constexpr NodeId node_id() const {
        return NodeId::from_bits(nodeid_);
    }



private:
    uint16_t nodeid_:7;
    uint16_t fcode_:4;
    uint16_t __resv__ : 5;

    constexpr CobId(const uint16_t bits){
        (*this) = std::bit_cast<CobId>(bits);
    }
};

// static constexpr CobId SYNC_COBID = CobId::from_bits(0x080);
// static constexpr CobId EMCY_COBID = CobId::from_bits(0x080);
// static constexpr CobId TIME_COBID = CobId::from_bits(0x100);


static_assert(sizeof(CobId) == sizeof(uint16_t));



// 0000h Reserved保留 
// 0001h to 025Fh Data types数据类型 
// 0260h to 0FFFh Reserved保留 
// 1000h to 1FFFh Communication profile area通讯对象子协议区 
    // 1000h to 1029h General communication objects通用通讯对象 
    // 1200h to 12FFh SDO parameter objects SDO参数对象 
    // 1300h to 13FFh CANopen safety objects 安全对象 
    // 1400h to 1BFFh PDO parameter objects PDO参数对象 
    // 1F00h to 1F11h SDO manager objects SDO管理对象 
    // 1F20h to 1F27h Configuration manager objects配置管理对象 
    // 1F50h to 1F54h Program control object程序控制对象 
    // 1F80h to 1F89h NMT master objects网络管理主机对象
// 2000h to 5FFFh Manufacturer-specific profile area制造商特定子协议区 
// 6000h to 9FFFh Standardized profile area标准化设备子协议区 
// A000h to AFFFh Network variables网络变量（符合IEC61131-3） 
// B000h to BFFFh System variables用于路由网关的系统变量 
// C000h to FFFFh Reserved保留
struct OdPreIndex{
    using Self = OdPreIndex;
    uint16_t count;

    static constexpr Self from_bits(const uint16_t bits){
        return Self{bits};
    }

    [[nodiscard]] constexpr uint16_t as_bits() const{
        return count;
    }

    [[nodiscard]] constexpr bool operator==(const Self & other) const{
        return count == other.count;
    }
};

static_assert(sizeof(OdPreIndex) == sizeof(uint16_t));

struct OdSubIndex{
    using Self = OdSubIndex;
    uint8_t count;

    static constexpr Self from_bits(const uint8_t bits){
        return Self{bits};
    }

    [[nodiscard]] constexpr uint8_t as_bits() const{
        return count;
    }

    [[nodiscard]] constexpr bool operator==(const Self & other) const{
        return count == other.count;
    }
};

static_assert(sizeof(OdSubIndex) == sizeof(uint8_t));   
// enum class [[nodiscard]] OdPreIndex:uint16_t{};
// enum class [[nodiscard]] OdSubIndex:uint8_t{};

struct [[nodiscard]] OdIndex{
    OdPreIndex pre;
    OdSubIndex sub;
    constexpr bool operator==(const OdIndex& other) const = default;
};




// class SdoCommand {
// public:
//     // 位域结构体
//     using CommandSpecifier = SdoCommandSpecifier;
//     // 构造函数
//     SdoCommand(const hal::CanClassicMsg & msg) {
//         specifier = std::bit_cast<CommandSpecifier>(msg.payload_bytes()[0]);
//     }

//     auto type() const { return SdoCommandType(speci.command); }

// private:
//     CommandSpecifier specifier;
// };



}