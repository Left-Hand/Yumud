#pragma once

#include "core/stream/ostream.hpp"

namespace ymd{

enum class EtherType:uint16_t{
    Ipv4 = 0x0800,
    Arp = 0x0806,
    Ipv6 = 0x86DD
};

inline OutputStream & operator<<(OutputStream & os, const EtherType & self){
    switch(self){
        case EtherType::Ipv4: return os << "Ipv4";
        case EtherType::Arp: return os << "Arp";
        case EtherType::Ipv6: return os << "Ipv6";
    }
    __builtin_unreachable();
}


struct [[nodiscard]] MacAddress final {
public:
    using Self = MacAddress;
    using Storage = std::array<uint8_t, 6>;

    // 本地管理地址的判断掩码（第1字节的次低位为1）
    static constexpr uint8_t LOCAL_ADMIN_MASK = 0x02;
    // 组播地址的判断掩码（第1字节的最低位为1）
    static constexpr uint8_t MULTICAST_MASK = 0x01;

    // ========== 构造函数（保持原有逻辑，补充显式构造） ==========
    constexpr MacAddress(const std::span<const uint8_t, 6> bytes) {
        std::copy(bytes.begin(), bytes.end(), bytes_.begin());
    }

    // 显式构造：从数组直接初始化
    explicit constexpr MacAddress(const Storage& bytes) : bytes_(bytes) {}

    // ========== 静态构造方法（保持原有） ==========
    static constexpr Self from_bytes(const std::span<const uint8_t, 6> bytes) {
        return Self(bytes);
    }

    // ========== 安全的下标访问（添加越界断言） ==========
    [[nodiscard]] constexpr uint8_t operator[](const size_t index) const {
        if(index >= size())
            __builtin_trap();
        return bytes_[index];
    }

    [[nodiscard]] constexpr uint8_t& operator[](const size_t index) {
        if(index >= size())
            __builtin_trap();
        return bytes_[index];
    }

    // ========== 大小获取（保持原有） ==========
    static constexpr size_t size() noexcept {
        return Storage{}.size();
    }

    // ========== 转为字节视图（保持原有） ==========
    [[nodiscard]] constexpr std::span<const uint8_t, 6> as_bytes() const noexcept {
        return std::span<const uint8_t, 6>(bytes_);
    }

    // ========== 核心功能：地址类型判断（对应 smoltcp 的方法） ==========
    /// 是否为广播地址（全 0xFF）
    [[nodiscard]] constexpr bool is_broadcast() const noexcept {
        for(size_t i = 0; i < 6; i++){
            if(bytes_[i] != 0xFF) return false;
        }
        return true;
    }

    /// 是否为组播地址（第1字节最低位为 1）
    [[nodiscard]] constexpr bool is_multicast() const noexcept {
        return (bytes_[0] & MULTICAST_MASK) != 0;
    }

    /// 是否为本地管理地址（第1字节次低位为 1）
    [[nodiscard]] constexpr bool is_local() const noexcept {
        return (bytes_[0] & LOCAL_ADMIN_MASK) != 0;
    }

    /// 是否为单播地址（非广播、非组播）
    [[nodiscard]] constexpr bool is_unicast() const noexcept {
        return !is_broadcast() && !is_multicast();
    }

    // ========== 比较运算符（用于判断广播地址） ==========
    [[nodiscard]] constexpr bool operator==(const Self& rhs) const noexcept = default;
    [[nodiscard]] constexpr bool operator!=(const Self& rhs) const noexcept = default;

private:
    Storage bytes_;

    // ========== 格式化输出（优化流操作，兼容 smoltcp 的 XX:XX:XX:XX:XX:XX 格式） ==========
    friend OutputStream& operator<<(OutputStream& os, const Self& self) {
        os << self[0];
        for (size_t i = 1; i < Self::size(); ++i) {
            os << os.brackets<':'>() << self[i];
        }
        return os;
    }
};
}