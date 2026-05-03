#include "backend_ops.hpp"

namespace{

[[nodiscard]] static constexpr uintptr_t upround_addr_aligned(const uintptr_t addr){
    static constexpr uintptr_t NUM = sizeof(size_t) - 1;
    static constexpr uintptr_t MASK = ~NUM;

    return (addr + NUM) & MASK;
}


static_assert(upround_addr_aligned(0) == 0);
static_assert(upround_addr_aligned(1) == 4);
static_assert(upround_addr_aligned(2) == 4);
static_assert(upround_addr_aligned(3) == 4);
static_assert(upround_addr_aligned(4) == 4);

template<typename T>
[[nodiscard]] static T * upround_ptr_aligned(T * ptr){
    const uintptr_t ret_ptr = upround_addr_aligned(reinterpret_cast<uintptr_t>(ptr));
    return reinterpret_cast<T *>(ret_ptr);
}

template<typename T>
[[nodiscard]] static bool ptr_is_aligned(const T * ptr){
    static constexpr uintptr_t NUM = sizeof(size_t) - 1;
    return reinterpret_cast<uintptr_t>(ptr) & NUM;
}

}

namespace nvcv2::backend{

void bitwise_not_inplace_softw(uint8_t * dst, const size_t len){
    if (len == 0) return;

    uint8_t * end = dst + len;
    constexpr size_t UNIT = sizeof(size_t);
    constexpr uintptr_t ALIGN_MASK = UNIT - 1;

    // 1. Process head until aligned
    while ((reinterpret_cast<uintptr_t>(dst) & ALIGN_MASK) != 0 && dst < end) {
        *dst = ~(*dst);
        ++dst;
    }

    // 2. Process aligned middle part in size_t chunks
    size_t * dst_word = reinterpret_cast<size_t *>(dst);
    size_t * end_word = reinterpret_cast<size_t *>(upround_ptr_aligned(end));

    while (dst_word < end_word) {
        *dst_word = ~(*dst_word);
        ++dst_word;
    }

    // 3. Process tail bytes
    dst = reinterpret_cast<uint8_t *>(dst_word);
    while (dst < end) {
        *dst = ~(*dst);
        ++dst;
    }
}


}