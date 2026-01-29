#pragma once


#include "core/string/view/string_view.hpp"

namespace ymd::str{
namespace detail {
    // 辅助函数：计算汉明距离
    static constexpr uint32_t hamming_distance(uint32_t a, uint32_t b) noexcept {
        return __builtin_popcount(a ^ b);
    }
    
    // 字符哈希函数（FNV-1a变体）
    static constexpr uint32_t hamming_char_hash(char c, uint32_t seed = 2166136261U) noexcept {
        constexpr uint32_t prime = 16777619U;
        uint32_t hash = seed;
        hash ^= static_cast<uint8_t>(c);
        hash *= prime;
        return hash;
    }
}

static constexpr uint32_t simhash_fingerprint(StringView str) noexcept {
    // 32位向量，初始化为0
    std::array<int16_t, 32> vector = {0};
    
    // 为每个字符更新向量
    for (char c : str) {
        // 为每个字符生成一个32位的伪随机位模式
        uint32_t hash = detail::hamming_char_hash(c);
        
        // 更新向量：位为1则加1，位为0则减1
        for (size_t i = 0; i < 32; ++i) {
            if (hash & (1u << i)) {
                vector[i] += 1;
            } else {
                vector[i] -= 1;
            }
        }
    }
    
    // 生成指纹：向量元素大于0则对应位为1
    uint32_t fingerprint = 0;
    for (size_t i = 0; i < 32; ++i) {
        if (vector[i] > 0) {
            fingerprint |= (1u << i);
        }
    }
    
    return fingerprint;
}

static constexpr float simhash_similarity(StringView str1, StringView str2) noexcept {
    const uint32_t fp1 = simhash_fingerprint(str1);
    const uint32_t fp2 = simhash_fingerprint(str2);
    
    // 计算汉明距离
    const uint32_t distance = detail::hamming_distance(fp1, fp2);
    
    // 归一化到 [0, 1]，0表示完全不同，1表示完全相同
    return 1.0f - (static_cast<float>(distance) / 32.0f);
}

// 重载版本：直接返回汉明距离
static constexpr int simhash_distance(StringView str1, StringView str2) noexcept {
    return detail::hamming_distance(simhash_fingerprint(str1), simhash_fingerprint(str2));
}

}