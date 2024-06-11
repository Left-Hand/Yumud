#ifndef __HASH_FUNCS__

#define __HASH_FUNCS__

constexpr uint32_t hash_impl(char const * str , size_t size){
    uint32_t hash = 5381;

    for (size_t i = 0; i < size; i++) {
        hash = ((hash << 5) + hash) ^ str[i]; /* hash * 33 + c */
    }

    return hash;
}

constexpr uint32_t operator "" _ha(char const* p, size_t size)  {
    return hash_impl(p, size);
}

#endif