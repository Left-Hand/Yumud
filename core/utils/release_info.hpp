#pragma once

#include "core/utils/build_date.hpp"
#include "core/utils/author.hpp"
#include "core/utils/serde.hpp"

namespace ymd{


// DERIVE_SERIALIZE_AS_TUPLE(Author)

struct ReleaseVersion final{
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
constexpr Hasher<S> & operator << (Hasher<S> & hs, const ReleaseVersion & self){
    return hs << self.major << self.minor;
}

template<>
struct serde::SerializeIterMaker<serde::RawBytes, ReleaseVersion>{
    static constexpr auto make(const ReleaseVersion & version){
        return make_serialize_iter<serde::RawBytes>(
            std::make_tuple(version.major, version.minor));
    }
};

struct ReleaseInfo{
    Author author;
    ReleaseVersion version;
    Date date;
    Time time;

    // Compile-time creation with validation
    static constexpr Option<ReleaseInfo> from(
        const char* author_name,
        ReleaseVersion version,
        Date date = Date::from_compiler(),
        Time time = Time::from_compiler()
    ) {
        const auto may_author = Author::from(author_name);
        if(may_author.is_none()) return None;
        return Some(ReleaseInfo{
            may_author.unwrap(), 
            version, 
            date, 
            time
        });
    }
};

OutputStream & operator <<(OutputStream & os, const ReleaseInfo & self){
    os << os.scoped("release")(
        os 
        << os.field("author")(os << self.author)
        << os.field("version")(os << self.version)
        << os.field("date")(os << self.date)
        << os.field("time")(os << self.time)
    );
    return os;
}

template<HashAlgo S>
constexpr Hasher<S> & operator << (Hasher<S> & hs, const ReleaseInfo & self){
    return hs << self.author << self.version << self.date << self.time;
}


template<>
struct serde::SerializeIterMaker<serde::RawBytes, ReleaseInfo>{
    static constexpr auto make(const ReleaseInfo & info){
        return serde::make_serialize_iter<serde::RawBytes>(
            std::make_tuple(info.author, info.version, info.date, info.time));
    }
};

}