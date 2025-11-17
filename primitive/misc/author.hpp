#pragma once

#include "core/utils/Option.hpp"
#include "core/string/string_view.hpp"
#include "core/utils/serde.hpp"
#include <utility>

// DERIVE_SERIALIZE_AS_TUPLE(Date)
namespace ymd{
struct Author final{
    static constexpr size_t MAX_NAME_LEN = 8;
    static constexpr Option<Author> from(const char * name){
        const auto slen = strlen(name);
        if(slen >= MAX_NAME_LEN) return None;
        return Some(Author{name});
    }

    constexpr StringView name() const{
        return StringView(name_.data(), 8);
    }

private:
    constexpr Author(const char * name){
        name_.fill(0);
        for(size_t i = 0; i < MAX_NAME_LEN && name[i]; ++i){
            name_[i] = name[i];
        }
    }
    std::array<char, MAX_NAME_LEN> name_;
};

OutputStream & operator<<(OutputStream & os, const Author & self){ 
    return os << "name: " << self.name();
}

template<HashAlgo S>
constexpr Hasher<S> & operator << (Hasher<S> & hs, const Author & self){
    return hs << self.name();
}

template<>
struct serde::SerializeGeneratorFactory<serde::RawLeBytes, Author>{
    static constexpr auto from(const Author & author){
        return make_serialize_generator<serde::RawLeBytes>(author.name());
    }
};
}