#ifndef __HASH_FUNCS__

#define __HASH_FUNCS__

struct Hasher{
public:
    __inline static constexpr uint32_t hash_uiml32(char const*data, size_t length){
        uint32_t h = 0;  
        uint16_t strLength = length, alignedLen = strLength / sizeof(uint32_t);
        for(uint16_t i = 0; i < alignedLen; ++i)  
            h = (h << 5) - h + ((uint32_t*)data)[i]; 
        for(uint16_t i = alignedLen << 2; i < strLength; ++i)
            h = (h << 5) - h + data[i]; 
        return h; 
    }

    __inline static constexpr uint32_t hash_fnv1a(char const*data, size_t length){
        uint32_t prime = 16777619U;
        uint32_t ret = 2166136261U;
        for (size_t i = 0; i < length; i++) {
            ret ^= (uint32_t)(data[i]);
            ret *= prime;
        }
        return ret;
    }


    __inline static constexpr uint32_t hash_djb(char const* str , size_t size){
        uint32_t hash = 5381;

        for (size_t i = 0; i < size; i++) {
            hash = ((hash << 5) + hash) ^ str[i]; /* hash * 33 + c */
        }

        return hash;
    }
};


template<typename T>
__inline constexpr uint32_t hash_impl(T const * data, size_t size){
    return Hasher::hash_djb((char const *)(data), sizeof(T) * size);
}

template<typename T>
__inline constexpr uint32_t hash_impl(const T & obj ){
    return hash_impl((&obj), (size_t)sizeof(obj));
}


__inline constexpr uint32_t operator "" _ha(char const* p, const size_t size)  {
    return hash_impl(p, size);
}

#endif