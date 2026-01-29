#pragma once

#include "core/string/view/string_view.hpp"

namespace ymd::str{

static constexpr size_t levenshtein_distance(StringView str1, StringView str2) noexcept{
    const size_t len1 = str1.length();
    const size_t len2 = str2.length();
    
    // 特殊情况处理
    if (len1 == 0) return static_cast<float>(len2);
    if (len2 == 0) return static_cast<float>(len1);
    
    // 使用单个数组优化
    constexpr size_t MAX_LEN = 32;
    
    // 确保字符串长度在合理范围内
    if (len1 > MAX_LEN || len2 > MAX_LEN) {
        // 返回一个合理的估计值
        return static_cast<float>(std::max(len1, len2));
    }
    
    // 总是使用较短的字符串作为内循环
    const auto [s_short, s_long] = [&] -> std::tuple<StringView, StringView>{
        if (len1 < len2) {
            return {str1, str2};
        } else {
            return {str2, str1};
        }
    }();
    
    const size_t short_len = s_short.length();
    const size_t long_len = s_long.length();
    
    // 使用一维数组，大小为较短字符串长度+1
    std::array<uint8_t, MAX_LEN> distances;
    
    // 初始化（相当于编辑距离的第一行）
    for (size_t i = 0; i <= short_len; ++i) {
        distances[i] = static_cast<uint8_t>(i);
    }
    
    // 动态规划计算
    for (size_t i = 1; i <= long_len; ++i) {
        uint8_t prev_diagonal = distances[0];
        uint8_t prev_above = static_cast<uint8_t>(i);
        distances[0] = prev_above;
        
        for (size_t j = 1; j <= short_len; ++j) {
            uint8_t old_diagonal = distances[j];
            
            // 计算成本
            uint8_t cost = (s_long[i - 1] == s_short[j - 1]) ? 0 : 1;
            
            // 计算最小值
            uint8_t deletion = distances[j] + 1;
            uint8_t insertion = distances[j - 1] + 1;
            uint8_t substitution = prev_diagonal + cost;
            
            distances[j] = std::min({deletion, insertion, substitution});
            
            prev_diagonal = old_diagonal;
        }
    }
    
    return static_cast<size_t>(distances[short_len]);
}
}