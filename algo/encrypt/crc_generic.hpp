#pragma once

namespace ymd::crc{

#include <cstdint>
#include <array>
#include <span>
#include <type_traits>

// ================================
// CRC 策略基类和具体实现
// ================================

// CRC 算法类型枚举
enum class CrcType {
    CRC8,
    CRC16,
    CRC32
};

// 通用 CRC 策略接口
template<CrcType Type>
struct CrcPolicy;

// ================================
// CRC-8 策略实现
// ================================

// 常用 CRC-8 算法预定义
namespace crc8_policies {
    // CRC-8 (标准)
    struct Standard {
        static constexpr uint8_t POLINOMIAL = 0x07;
        static constexpr uint8_t INITIAL_VALUE = 0x00;
        static constexpr bool REFIN = false;
        static constexpr bool REFOUT = false;
        static constexpr uint8_t XOROUT = 0x00;
    };
    
    // CRC-8/ROHC
    struct Rohc {
        static constexpr uint8_t POLINOMIAL = 0x07;
        static constexpr uint8_t INITIAL_VALUE = 0xFF;
        static constexpr bool REFIN = true;
        static constexpr bool REFOUT = true;
        static constexpr uint8_t XOROUT = 0x00;
    };
    
    // CRC-8/MAXIM
    struct Maxim {
        static constexpr uint8_t POLINOMIAL = 0x31;
        static constexpr uint8_t INITIAL_VALUE = 0x00;
        static constexpr bool REFIN = true;
        static constexpr bool REFOUT = true;
        static constexpr uint8_t XOROUT = 0x00;
    };
}

// ================================
// CRC-16 策略实现
// ================================

namespace crc16_policies {
    // CRC-16/IBM (ARC)
    struct Ibm {
        static constexpr uint16_t POLINOMIAL = 0x8005;
        static constexpr uint16_t INITIAL_VALUE = 0x0000;
        static constexpr bool REFIN = true;
        static constexpr bool REFOUT = true;
        static constexpr uint16_t XOROUT = 0x0000;
    };
    
    // CRC-16/MODBUS (这就是你提供的代码对应的算法)
    struct Modbus {
        static constexpr uint16_t POLINOMIAL = 0x8005;
        static constexpr uint16_t INITIAL_VALUE = 0xFFFF;
        static constexpr bool REFIN = true;
        static constexpr bool REFOUT = true;
        static constexpr uint16_t XOROUT = 0x0000;
    };
    
    // CRC-16/CCITT
    struct Ccitt {
        static constexpr uint16_t POLINOMIAL = 0x1021;
        static constexpr uint16_t INITIAL_VALUE = 0xFFFF;
        static constexpr bool REFIN = false;
        static constexpr bool REFOUT = false;
        static constexpr uint16_t XOROUT = 0x0000;
    };
    
    // CRC-16/XMODEM
    struct Xmodem {
        static constexpr uint16_t POLINOMIAL = 0x1021;
        static constexpr uint16_t INITIAL_VALUE = 0x0000;
        static constexpr bool REFIN = false;
        static constexpr bool REFOUT = false;
        static constexpr uint16_t XOROUT = 0x0000;
    };
}


// ================================
// CRC-32 策略实现
// ================================

namespace crc32_policies {
    // CRC-32 (Ethernet, ZIP等)
    struct Standard {
        static constexpr uint32_t POLINOMIAL = 0x04C11DB7;
        static constexpr uint32_t INITIAL_VALUE = 0xFFFFFFFF;
        static constexpr bool REFIN = true;
        static constexpr bool REFOUT = true;
        static constexpr uint32_t XOROUT = 0xFFFFFFFF;
    };
    
    // CRC-32C (Castagnoli)
    struct Castagnoli {
        static constexpr uint32_t POLINOMIAL = 0x1EDC6F41;
        static constexpr uint32_t INITIAL_VALUE = 0xFFFFFFFF;
        static constexpr bool REFIN = true;
        static constexpr bool REFOUT = true;
        static constexpr uint32_t XOROUT = 0xFFFFFFFF;
    };
}

// 位反转函数
[[nodiscard]] static constexpr uint8_t reverse_8bits(uint8_t x) {
    x = ((x & 0x55) << 1) | ((x & 0xAA) >> 1);
    x = ((x & 0x33) << 2) | ((x & 0xCC) >> 2);
    x = (x << 4) | (x >> 4);
    return x;
}

// 16位位反转
[[nodiscard]] static constexpr uint16_t reverse_16bits(uint16_t x) {
    x = ((x & 0x5555) << 1) | ((x & 0xAAAA) >> 1);
    x = ((x & 0x3333) << 2) | ((x & 0xCCCC) >> 2);
    x = ((x & 0x0F0F) << 4) | ((x & 0xF0F0) >> 4);
    x = ((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8);
    return x;
}

// 32位位反转
[[nodiscard]] static constexpr uint32_t reverse_32bits(uint32_t x) {
    x = ((x & 0x55555555) << 1) | ((x & 0xAAAAAAAA) >> 1);
    x = ((x & 0x33333333) << 2) | ((x & 0xCCCCCCCC) >> 2);
    x = ((x & 0x0F0F0F0F) << 4) | ((x & 0xF0F0F0F0) >> 4);
    x = ((x & 0x00FF00FF) << 8) | ((x & 0xFF00FF00) >> 8);
    x = (x << 16) | (x >> 16);
    return x;
}

// CRC-8 策略模板
template<typename Params>
struct Crc8Policy {
    using value_type = uint8_t;
    static constexpr CrcType type = CrcType::CRC8;
    
    // 策略参数
    static constexpr uint8_t POLINOMIAL = Params::POLINOMIAL;
    static constexpr uint8_t INITIAL_VALUE = Params::INITIAL_VALUE;
    static constexpr bool REFIN = Params::REFIN;
    static constexpr bool REFOUT = Params::REFOUT;
    static constexpr uint8_t XOROUT = Params::XOROUT;
    
    // 表生成
    static constexpr std::array<uint8_t, 256> generate_table() {
        std::array<uint8_t, 256> table{};
        for (uint16_t i = 0; i < 256; ++i) {
            uint8_t crc = static_cast<uint8_t>(i);
            for (uint8_t j = 0; j < 8; ++j) {
                if (crc & 0x80) {
                    crc = (crc << 1) ^ POLINOMIAL;
                } else {
                    crc <<= 1;
                }
            }
            
            if constexpr (REFIN) {
                // 反射输入，需要反射表项
                table[i] = reverse_8bits(crc);
            } else {
                table[i] = crc;
            }
        }
        return table;
    }
    
private:

};


template<typename Params>
struct Crc16Policy {
    using value_type = uint16_t;
    static constexpr CrcType type = CrcType::CRC16;
    
    // 策略参数
    static constexpr uint16_t POLINOMIAL = Params::POLINOMIAL;
    static constexpr uint16_t INITIAL_VALUE = Params::INITIAL_VALUE;
    static constexpr bool REFIN = Params::REFIN;
    static constexpr bool REFOUT = Params::REFOUT;
    static constexpr uint16_t XOROUT = Params::XOROUT;
    
    // 表生成 - 单表实现
    static constexpr std::array<uint16_t, 256> generate_table() {
        std::array<uint16_t, 256> table{};
        
        for (uint16_t i = 0; i < 256; ++i) {
            uint16_t crc = static_cast<uint16_t>(i);
            
            if constexpr (REFIN) {
                crc = reverse_16bits(crc) >> 8;  // 对于反射，输入字节需要先反射
            } else {
                crc <<= 8;
            }
            
            for (uint8_t j = 0; j < 8; ++j) {
                if (crc & 0x8000) {
                    crc = (crc << 1) ^ POLINOMIAL;
                } else {
                    crc <<= 1;
                }
            }
            
            table[i] = crc;
        }
        return table;
    }
    
    // 双表生成（兼容你原来的代码结构）
    struct DoubleTable {
        std::array<uint8_t, 256> hi;  // 高字节表
        std::array<uint8_t, 256> lo;  // 低字节表
    };
    
    static constexpr DoubleTable generate_double_table() {
        DoubleTable tables{};
        auto full_table = generate_table();
        
        for (size_t i = 0; i < 256; ++i) {
            tables.hi[i] = static_cast<uint8_t>(full_table[i] >> 8);
            tables.lo[i] = static_cast<uint8_t>(full_table[i] & 0xFF);
        }
        return tables;
    }
    
private:

};

template<typename Params>
struct Crc32Policy {
    using value_type = uint32_t;
    static constexpr CrcType type = CrcType::CRC32;
    
    // 策略参数
    static constexpr uint32_t POLINOMIAL = Params::POLINOMIAL;
    static constexpr uint32_t INITIAL_VALUE = Params::INITIAL_VALUE;
    static constexpr bool REFIN = Params::REFIN;
    static constexpr bool REFOUT = Params::REFOUT;
    static constexpr uint32_t XOROUT = Params::XOROUT;
    
    // 表生成
    static constexpr std::array<uint32_t, 256> generate_table() {
        std::array<uint32_t, 256> table{};
        
        for (uint32_t i = 0; i < 256; ++i) {
            uint32_t crc = i;
            
            if constexpr (REFIN) {
                crc = reverse_32bits(crc) >> 24;
            } else {
                crc <<= 24;
            }
            
            for (uint8_t j = 0; j < 8; ++j) {
                if (crc & 0x80000000) {
                    crc = (crc << 1) ^ POLINOMIAL;
                } else {
                    crc <<= 1;
                }
            }
            
            table[i] = crc;
        }
        return table;
    }
    
private:

};

// ================================
// CRC 累加器主模板
// ================================

template<typename Policy>
class CrcAccumulator {
public:
    using value_type = typename Policy::value_type;
    
    // 编译时表生成
    static constexpr auto TABLE = Policy::generate_table();
    
private:
    value_type crc_;
    
    // 反射函数（编译时）
    static constexpr value_type reflect(value_type x, uint8_t bits) {
        value_type reflection = 0;
        for (uint8_t i = 0; i < bits; ++i) {
            if (x & (static_cast<value_type>(1) << i)) {
                reflection |= static_cast<value_type>(1) << ((bits - 1) - i);
            }
        }
        return reflection;
    }
    
    // 字节反射（根据位宽）
    static constexpr value_type reflect_byte(uint8_t byte) {
        if constexpr (Policy::REFIN) {
            return reflect(byte, 8);
        } else {
            return byte;
        }
    }
    
public:
    // 构造函数
    constexpr CrcAccumulator() : crc_(Policy::INITIAL_VALUE) {}
    
    // 显式初始化
    constexpr explicit CrcAccumulator(value_type initial_value) 
        : crc_(initial_value) {}
    
    // 处理单个字节
    constexpr void push_byte(uint8_t byte) {
        uint8_t index;
        
        if constexpr (Policy::REFIN) {
            // 对于反射算法
            index = static_cast<uint8_t>(crc_ ^ reflect_byte(byte));
        } else {
            // 对于非反射算法
            index = static_cast<uint8_t>((crc_ >> (sizeof(value_type) * 8 - 8)) ^ byte);
        }
        
        if constexpr (std::is_same_v<value_type, uint8_t>) {
            // CRC-8
            crc_ = TABLE[index];
        } else if constexpr (std::is_same_v<value_type, uint16_t>) {
            // CRC-16
            crc_ = (crc_ << 8) ^ TABLE[index];
        } else {
            // CRC-32
            crc_ = (crc_ << 8) ^ TABLE[index];
        }
    }
    
    // 处理字节块
    constexpr void push_bytes(std::span<const uint8_t> bytes) {
        for (uint8_t byte : bytes) {
            push_byte(byte);
        }
    }
    
    // 获取当前 CRC 值
    [[nodiscard]] constexpr value_type get() const {
        value_type result = crc_;
        
        // 应用输出反射
        if constexpr (Policy::REFOUT) {
            result = reflect(result, sizeof(value_type) * 8);
        }
        
        // 应用最终异或
        result ^= Policy::XOROUT;
        
        return result;
    }
    
    // 重置 CRC 值
    constexpr void reset() {
        crc_ = Policy::INITIAL_VALUE;
    }
    
    // 重置为指定值
    constexpr void reset(value_type value) {
        crc_ = value;
    }
};

// ================================
// 双表版本 CRC-16 累加器（兼容原有接口）
// ================================

template<typename Policy>
class Crc16DoubleTableAccumulator {
    static_assert(std::is_same_v<typename Policy::value_type, uint16_t>, "This class only works for CRC-16");
    
private:
    // 编译时生成双表
    static constexpr auto TABLES = Policy::generate_double_table();
    
    uint8_t crclo_;
    uint8_t crchi_;
    
public:
    constexpr Crc16DoubleTableAccumulator() 
        : crclo_(static_cast<uint8_t>(Policy::INITIAL_VALUE & 0xFF))
        , crchi_(static_cast<uint8_t>(Policy::INITIAL_VALUE >> 8)) {}
    
    constexpr void push_byte(uint8_t byte) {
        // 处理反射输入
        uint8_t input_byte;
        if constexpr (Policy::REFIN) {
            input_byte = static_cast<uint8_t>(
                (byte & 0x01) << 7 | (byte & 0x02) << 5 |
                (byte & 0x04) << 3 | (byte & 0x08) << 1 |
                (byte & 0x10) >> 1 | (byte & 0x20) >> 3 |
                (byte & 0x40) >> 5 | (byte & 0x80) >> 7);
        } else {
            input_byte = byte;
        }
        
        const uint8_t index = crclo_ ^ input_byte;
        crclo_ = crchi_ ^ TABLES.hi[index];
        crchi_ = TABLES.lo[index];
    }
    
    constexpr void push_bytes(std::span<const uint8_t> bytes) {
        for (uint8_t byte : bytes) {
            push_byte(byte);
        }
    }
    
    [[nodiscard]] constexpr uint16_t get() const {
        uint16_t result = (static_cast<uint16_t>(crchi_) << 8) | crclo_;
        
        // 应用输出反射
        if constexpr (Policy::REFOUT) {
            result = static_cast<uint16_t>(
                (result & 0x0001) << 15 | (result & 0x0002) << 13 |
                (result & 0x0004) << 11 | (result & 0x0008) << 9 |
                (result & 0x0010) << 7 | (result & 0x0020) << 5 |
                (result & 0x0040) << 3 | (result & 0x0080) << 1 |
                (result & 0x0100) >> 1 | (result & 0x0200) >> 3 |
                (result & 0x0400) >> 5 | (result & 0x0800) >> 7 |
                (result & 0x1000) >> 9 | (result & 0x2000) >> 11 |
                (result & 0x4000) >> 13 | (result & 0x8000) >> 15);
        }
        
        // 应用最终异或
        result ^= Policy::XOROUT;
        
        return result;
    }
};

}