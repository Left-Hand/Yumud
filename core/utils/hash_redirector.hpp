#pragma once

#include "../utils/result.hpp"


namespace ymd{

template<size_t N>
struct [[nodiscard]] HashRedirector final {
    static_assert(N < 256);
    
    template<typename Mapper, typename ... Args>
    [[nodiscard]] constexpr Result<void, std::tuple<size_t, size_t>> 
    try_init(Mapper && hash_extractor, Args && ... entries) {
        static_assert(N == sizeof...(entries));

        // 使用提取器获取哈希值
        {
            size_t i = 0;
            ((hashes[i] = hash_extractor(entries), remapped_index[i] = static_cast<uint8_t>(i), ++i), ...);
        }
        
        // 排序
        for (size_t i = 0; i < N - 1; ++i) {
            for (size_t j = 0; j < N - i - 1; ++j) {
                if (hashes[j] > hashes[j + 1]) {
                    std::swap(hashes[j], hashes[j + 1]);
                    std::swap(remapped_index[j], remapped_index[j + 1]);
                }
            }
        }
        
        // 检查哈希碰撞
        for (size_t i = 0; i < N - 1; ++i) {
            if (hashes[i] == hashes[i + 1]) {
                return Err(std::make_tuple(remapped_index[i], remapped_index[i + 1]));
            }
        }

        return Ok();
    }

    constexpr Option<uint8_t> find(const uint32_t hash) const {
        size_t left = 0, right = N;
        while (left < right) {
            size_t mid = left + (right - left) / 2;
            if (hashes[mid] < hash) {
                left = mid + 1;
            } else {
                right = mid;
            }
        }
        
        if (left < N && hashes[left] == hash) {
            return Some(remapped_index[left]);
        }
        return None;
    }

    std::array<uint32_t, N> hashes{};
    std::array<uint8_t, N> remapped_index{};
};
}