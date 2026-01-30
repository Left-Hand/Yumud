#pragma once

#include "core/utils/Option.hpp"
#include "core/string/view/string_view.hpp"
#include "core/string/owned/char_array.hpp"
#include "core/utils/serde.hpp"
#include <utility>

// DERIVE_SERIALIZE_AS_TUPLE(Date)
namespace ymd{
struct [[nodiscard]] Author final{
    static constexpr size_t MAX_NAME_LEN = 8;
    static constexpr Option<Author> from_str(const StringView str){
        if(str.length() >= MAX_NAME_LEN) return None;
        return Some(Author{str});
    }

    constexpr StringView name() const{
        return StringView(name_.data(), 8);
    }

private:
    constexpr Author(const StringView str){
        name_.fill(0);
        for(size_t i = 0; i < MAX_NAME_LEN && str[i]; ++i){
            name_[i] = str[i];
        }
    }
    CharArray<MAX_NAME_LEN> name_;
};

OutputStream & operator<<(OutputStream & os, const Author & self){ 
    return os << os.field("name")(self.name());
}

template<HashAlgo S>
constexpr HashBuilder<S> & operator << (HashBuilder<S> & hs, const Author & self){
    return hs << self.name();
}

template<>
struct [[nodiscard]] serde::SerializeGeneratorFactory<serde::RawLeBytes, Author>{
    static constexpr auto from(const Author & author){
        return make_serialize_generator<serde::RawLeBytes>(author.name());
    }
};
}