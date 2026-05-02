#pragma once

#include <cstdint>



namespace ymd::mem{
void memcpy_dst_aligned4(__restrict__ void *dest, __restrict__ const void *source, size_t len){
    if (len == 0) return;
    
    // Check alignment
    if (reinterpret_cast<uintptr_t>(dest) % 4 != 0)
        __builtin_unreachable();
    
    uint32_t* d = static_cast<uint32_t*>(dest);
    const uint32_t* s = static_cast<const uint32_t*>(source);
    
    size_t full_words = len >> 2;  // len / 4
    size_t leftover = len & 3;     // len % 4
    
    if (full_words == 0) {
        uint8_t* d8 = static_cast<uint8_t*>(dest);
        const uint8_t* s8 = static_cast<const uint8_t*>(source);
        for(size_t i = 0; i < len; ++i) {
            d8[i] = s8[i];
        }
        return;
    }
    
    // Use Duff's Device for word copying
    size_t n = (full_words + 7) / 8;
    
    switch (full_words % 8) {
        case 0: do { *d++ = *s++;
        case 7:      *d++ = *s++;
        case 6:      *d++ = *s++;
        case 5:      *d++ = *s++;
        case 4:      *d++ = *s++;
        case 3:      *d++ = *s++;
        case 2:      *d++ = *s++;
        case 1:      *d++ = *s++;
                } while (--n > 0);
    }
    
    // Handle remaining bytes correctly
    if (leftover) {
        uint8_t* d8 = reinterpret_cast<uint8_t*>(d);  // d already incremented
        const uint8_t* s8 = reinterpret_cast<const uint8_t*>(s);  // s already incremented
        for(size_t i = 0; i < leftover; ++i) {
            d8[i] = s8[i];
        }
    }
}

void memcpy_src_aligned4(__restrict__ void *dest, __restrict__ const void *source, size_t len){
    if (len == 0) return;
    
    // Check alignment
    if (reinterpret_cast<uintptr_t>(source) % 4 != 0)
        __builtin_unreachable();
    
    uint8_t* d = static_cast<uint8_t*>(dest);
    const uint32_t* s = static_cast<const uint32_t*>(source);
    
    size_t full_words = len >> 2;  // len / 4
    size_t leftover = len & 3;     // len % 4
    
    // Copy full 32-bit words
    for(size_t i = 0; i < full_words; ++i) {
        uint32_t word = *s++;
        // Write the 32-bit word to destination byte by byte
        d[0] = static_cast<uint8_t>(word & 0xFF);
        d[1] = static_cast<uint8_t>((word >> 8) & 0xFF);
        d[2] = static_cast<uint8_t>((word >> 16) & 0xFF);
        d[3] = static_cast<uint8_t>((word >> 24) & 0xFF);
        d += 4;
    }
    
    // Handle remaining bytes
    if (leftover) {
        const uint8_t* s8 = reinterpret_cast<const uint8_t*>(s);
        for(size_t i = 0; i < leftover; ++i) {
            d[i] = s8[i];
        }
    }
}


#if 0
//TODO 提高memcmp的性能
__fast_inline constexpr int __ymd_memcmp(const void * str1, const void * str2, uint32_t len){
    for (uint32_t i = 0; i < len; i++) {
        if ((static_cast<const uint8_t*>(str1))[i] < (static_cast<const uint8_t*>(str2))[i]) {
            return -1;
        } else if ((static_cast<const uint8_t*>(str1))[i] > (static_cast<const uint8_t*>(str2))[i]) {
            return 1;
        }
    }
    return 0;
}

#define memcmp(str1, str2, len) __ymd_memcmp(str1, str2, len)

//TODO 提高memset的性能
__fast_inline constexpr void * __ymd_memset(void * dest, const char c, const uint32_t len){
	if(std::is_constant_evaluated()){
		return __builtin_memset(dest, c, len);
	}

    for(uint32_t i = 0; i < len; i++){
        reinterpret_cast<char *>(dest)[i] = c;
    }

    return dest;
}

#define memset(source, c, len) __ymd_memset(source, c, len)


//TODO 提高memcpy的性能
__fast_inline constexpr void * __ymd_memcpy(void * dest, const void * src, const uint32_t len){
	if(std::is_constant_evaluated()){
		return __builtin_memcpy(dest, src, len);
	}

    for (uint32_t i = 0; i < len; ++i) {
        (static_cast<uint8_t*>(dest))[i] = (static_cast<const uint8_t*>(src))[i];
    }
    return dest;
}

#define memcpy(dest, source, len) __ymd_memcpy(dest, source, len)

//TODO 提高strlen的性能
__fast_inline constexpr size_t __ymd_strlen(const char * str){
	if(std::is_constant_evaluated()){
		return __builtin_strlen(str);
	}
	
	size_t len = 0;
	while (str[len] != '\0') {
		++len;
	}
	return len;
}

#define strlen(str) __ymd_strlen(str)


//TODO 提高strcpy的性能
__fast_inline constexpr char * __ymd_strcpy(char * dest, const char * src){
    for (; *src != '\0'; ++src, ++dest) {
        *dest = *src;
    }
    return dest;
}

#define strcpy(d, s) __ymd_strcpy(d, s)

#endif
}