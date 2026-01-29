#pragma once

#include "core/string/view/string_view.hpp"

namespace ymd::str{


static constexpr float ngram_similarity(StringView str1, StringView str2, size_t n = 2) noexcept {
    // 限制n的最大值以防止栈溢出和无效计算
    if (n > 8 || n == 0) return 0.0f;
    
    const size_t len1 = str1.length();
    const size_t len2 = str2.length();
    
    // 如果字符串长度小于n，使用回退策略
    if (len1 < n || len2 < n) {
        if (len1 == 0 && len2 == 0) return 1.0f;
        if (len1 == 0 || len2 == 0) return 0.0f;
        
        // 使用位图而不是bool数组，更节省空间
        // 32字节 * 2 = 64字节
        std::array<uint32_t, 8> char_set1;
        std::array<uint32_t, 8> char_set2;
        
        char_set1.fill(0);
        char_set2.fill(0);

        // 构建字符集位图
        for (const char c : str1) {
            uint8_t idx = static_cast<uint8_t>(c);
            char_set1[idx >> 5] |= (1u << (idx & 0x1F));
        }
        for (const char c : str2) {
            uint8_t idx = static_cast<uint8_t>(c);
            char_set2[idx >> 5] |= (1u << (idx & 0x1F));
        }
        
        size_t intersection_size = 0;
        size_t union_size = 0;
        for (size_t i = 0; i < 8; ++i) {
            uint32_t inter = char_set1[i] & char_set2[i];
            uint32_t uni = char_set1[i] | char_set2[i];
            
            intersection_size += __builtin_popcount(inter);
            union_size += __builtin_popcount(uni);
        }
        
        return union_size > 0 ? static_cast<float>(intersection_size) / union_size : 0.0f;
    }else{
        const size_t ngram_count1 = len1 - n + 1;
        const size_t ngram_count2 = len2 - n + 1;
        
        // 估计哈希表大小，限制在较小范围内
        // 使用固定大小的位图而不是哈希表，更节省空间
        constexpr size_t MAX_NGRAM_COUNT = 256; // 限制最多处理256个不同的n-gram
        constexpr size_t BITMAP_SIZE = 256;     // 256位 = 32字节
        
        // 使用两个小位图来记录n-gram
        std::array<uint32_t, BITMAP_SIZE / 32> bitmap1;
        std::array<uint32_t, BITMAP_SIZE / 32> bitmap2;

        bitmap1.fill(0);
        bitmap2.fill(0);

        size_t common_count = 0;
        size_t count1 = 0;
        size_t count2 = 0;
        
        // 限制处理的n-gram数量，避免过多计算
        const size_t limit1 = std::min(ngram_count1, static_cast<size_t>(MAX_NGRAM_COUNT));
        const size_t limit2 = std::min(ngram_count2, static_cast<size_t>(MAX_NGRAM_COUNT));
        
        // 生成 str1 的 n-gram 并记录到bitmap1
        for (size_t i = 0; i < limit1; ++i) {
            // 使用简单的哈希函数生成0-255之间的值
            uint32_t hash = 0;
            for (size_t j = 0; j < n; ++j) {
                hash = (hash * 31) + static_cast<uint8_t>(str1[i + j]);
            }
            
            uint8_t bit_idx = hash & 0xFF; // 取低8位
            uint8_t word_idx = bit_idx >> 5;
            uint32_t bit_mask = 1u << (bit_idx & 0x1F);
            
            if (!(bitmap1[word_idx] & bit_mask)) {
                bitmap1[word_idx] |= bit_mask;
                ++count1;
            }
        }
        
        // 生成 str2 的 n-gram 并计算与str1的交集
        for (size_t i = 0; i < limit2; ++i) {
            uint32_t hash = 0;
            for (size_t j = 0; j < n; ++j) {
                hash = (hash * 31) + static_cast<uint8_t>(str2[i + j]);
            }
            
            uint8_t bit_idx = hash & 0xFF;
            uint8_t word_idx = bit_idx >> 5;
            uint32_t bit_mask = 1u << (bit_idx & 0x1F);
            
            // 检查是否在bitmap1中
            if ((bitmap1[word_idx] & bit_mask) && !(bitmap2[word_idx] & bit_mask)) {
                ++common_count;
            }
            
            // 记录到bitmap2
            if (!(bitmap2[word_idx] & bit_mask)) {
                bitmap2[word_idx] |= bit_mask;
                ++count2;
            }
        }
        
        // 计算 Sørensen-Dice 系数
        size_t den = count1 + count2;
        return float(2 * common_count) / den;
    }
    

}

}