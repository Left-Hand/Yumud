#pragma once

#include "canopen_funccode.hpp"
#include "core/container/bits_set.hpp"

namespace ymd::canopen::primitive{

using CanFrame = hal::BxCanFrame;
using CanPayload = hal::BxCanPayload;
using hal::CanStdId;
using namespace ymd::literals;

struct SubEntry;
struct CobId;

struct [[nodiscard]] NodeId final{

    // 1：NodeID由7bit组成，其中0是保留ID，代表一个未知的节点。
    // 2：Node ID取值为1-127，包含1-127，其中126，127是保留ID。
    // 3：NodeID分为SourceNodeID和DestinationNodeID。
    // 4：SourceNodeID表示节点自身的ID。
    // 5：DestinationNodeID表示对方的节点ID。
    // 6：只有Service帧才会有DestinationNodeID，需要应答。

    using Self = NodeId;

    uint8_t bits;

    static constexpr NodeId from_bits(const uint8_t bits){
        return NodeId{bits};
    }

    static constexpr NodeId from_b7(const Bs7 bs){
        return NodeId{bs.to_bits()};
    }

    static constexpr NodeId from_u7(const uint8_t bits){
        if(bits & 0b10000000) [[unlikely]] 
            __builtin_trap();
        return NodeId{static_cast<uint8_t>(bits & 0b1111111)};
    }

    static constexpr Option<NodeId> try_from_u7(const uint8_t bits){
        if(bits & 0b10000000) [[unlikely]]
            return None;
        return Some(NodeId{static_cast<uint8_t>(bits & 0b1111111)});
    }

    static constexpr NodeId boardcast(){
        return from_u7(0);
    }

    [[nodiscard]] constexpr uint8_t to_u7() const{
        return static_cast<uint8_t>(bits & 0x7f);
    }

    [[nodiscard]] constexpr Bs7 to_b7() const{
        return Bs7::from_bits_unchecked(bits);
    }

    //是否为广播地址
    [[nodiscard]] constexpr bool is_boardcast() const {
        return bits == 0;
    }

    //是否为保留地址
    [[nodiscard]] constexpr bool is_preserved() const{
        return bits == 244 || bits == 255;
    }

    [[nodiscard]] constexpr bool is_acceptable_with(const NodeId & other) const {
        return other.is_boardcast() || bits == other.bits;
    }

    // cobid + fcode
    [[nodiscard]] constexpr CobId with_func_code(const FunctionCode fcode) const;

    [[nodiscard]] constexpr bool operator==(const NodeId & other) const{
        return bits == other.bits;
    }
};



struct [[nodiscard]] CobId final{
    constexpr explicit CobId(const hal::CanStdId stdid){
        (*this) = std::bit_cast<CobId>(stdid.to_u11());
    }

    static constexpr CobId from_parts(
        const NodeId _nodeid, 
        const FunctionCode _fcode
    ){
        const uint16_t bits = static_cast<uint16_t>(
            _nodeid.to_u7() | 
            static_cast<uint16_t>(_fcode.to_u4() << 7));

        return from_bits(bits);
    }

    constexpr hal::CanStdId to_stdid() const {
        return hal::CanStdId::from_bits(to_bits());
    }

    constexpr FunctionCode func_code() const {
        return FunctionCode::from_bits(static_cast<uint8_t>(fcode_));
    }

    static constexpr CobId from_bits(const uint16_t bits){
        return std::bit_cast<CobId>(bits);
    }

    [[nodiscard]] constexpr uint16_t to_bits() const {
        return std::bit_cast<uint16_t>(*this);
    }

    static constexpr CobId from_u11(const uint16_t bits){
        if(bits & static_cast<uint16_t>(~CanStdId::MAX_VALUE)) [[unlikely]]
            __builtin_trap();
        return from_bits(bits);
    }

    [[nodiscard]] constexpr uint16_t to_u11() const {
        return std::bit_cast<uint16_t>(*this);
    }

    constexpr NodeId node_id() const {
        return NodeId::from_u7(nodeid_);
    }

private:
    uint16_t nodeid_:7;
    uint16_t fcode_:4;
    uint16_t __resv__ : 5;
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
struct [[nodiscard]] OdPreIndex final{
    using Self = OdPreIndex;
    uint16_t count;

    /// @brief 从比特位构造主序列，无任何检查
    /// @param bits 
    static constexpr Self from_bits(const uint16_t bits){
        return Self{bits};
    }

    [[nodiscard]] constexpr uint16_t to_bits() const{
        return count;
    }

    [[nodiscard]] constexpr bool operator==(const Self & other) const{
        return count == other.count;
    }
};

static_assert(sizeof(OdPreIndex) == sizeof(uint16_t));

struct [[nodiscard]] OdSubIndex final{
    using Self = OdSubIndex;
    uint8_t count;

    /// @brief 从比特位构造次序列，无任何检查
    /// @param bits 
    static constexpr Self from_bits(const uint8_t bits){
        return Self{bits};
    }

    [[nodiscard]] constexpr uint8_t to_bits() const{
        return count;
    }

    [[nodiscard]] constexpr bool operator==(const Self & other) const{
        return count == other.count;
    }
};

static_assert(sizeof(OdSubIndex) == sizeof(uint8_t));   


struct [[nodiscard]] OdIndex final{
    using Self = OdIndex;
    OdPreIndex pre;
    OdSubIndex sub;

    constexpr explicit OdIndex(const uint16_t _pre, const uint8_t _sub):
        pre(OdPreIndex::from_bits(_pre)),
        sub(OdSubIndex::from_bits(_sub)){;}

    static constexpr Self from_parts(const OdPreIndex _pre, const OdSubIndex _sub){
        return Self(_pre.to_bits(), _sub.to_bits());
    }
    [[nodiscard]] constexpr bool operator==(const OdIndex& other) const{
        return pre == other.pre and sub == other.sub;
    }
};


[[nodiscard]] constexpr CobId NodeId::with_func_code(const FunctionCode fcode) const{
    return CobId::from_parts(*this, fcode);
}


}