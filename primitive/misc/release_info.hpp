#pragma once

#include "primitive/misc/build_date.hpp"
#include "primitive/misc/author.hpp"
#include "core/utils/serde.hpp"

namespace ymd{


// DERIVE_SERIALIZE_AS_TUPLE(Author)

struct [[nodiscard]] ReleaseVersion final{
    uint8_t major;
    uint8_t minor;

    constexpr auto operator == (const ReleaseVersion & other) const {
        return (major == other.major) and (minor == other.minor);
    }

    constexpr auto operator<=>(const ReleaseVersion & other) const{
        if(major != other.major) return major <=> other.major;
        else return minor <=> other.minor;
    }
};

OutputStream & operator <<(OutputStream & os, const ReleaseVersion & self){
    return os << os.brackets<'{'>() 
            << self.major << os.brackets<':'>()
            << self.minor
            << os.brackets<'}'>()
        ;
}

template<HashAlgo S>
constexpr HashBuilder<S> & operator << (HashBuilder<S> & hs, const ReleaseVersion & self){
    return hs << self.major << self.minor;
}

template<>
struct [[nodiscard]] serde::SerializeGeneratorFactory<serde::RawLeBytes, ReleaseVersion>{
    static constexpr auto from(const ReleaseVersion & version){
        return make_serialize_generator<serde::RawLeBytes>(
            std::make_tuple(version.major, version.minor));
    }
};

struct [[nodiscard]] ReleaseInfo final{
    Author author;
    ReleaseVersion version;
    Date date = Date::from_compiler();
    Time time = Time::from_compiler();

};

OutputStream & operator <<(OutputStream & os, const ReleaseInfo & self){
    os << os.scoped("release")(os.field("author")(self.author)
        ,os.field("version")(self.version)
        ,os.field("date")(self.date)
        ,os.field("time")(self.time)
    );
    return os;
}

template<HashAlgo S>
constexpr HashBuilder<S> & operator << (HashBuilder<S> & hs, const ReleaseInfo & self){
    return hs << self.author << self.version << self.date << self.time;
}


template<>
struct [[nodiscard]] serde::SerializeGeneratorFactory<serde::RawLeBytes, ReleaseInfo>{
    static constexpr auto from(const ReleaseInfo & info){
        return serde::make_serialize_generator<serde::RawLeBytes>(
            std::make_tuple(info.author, info.version, info.date, info.time));
    }
};

}