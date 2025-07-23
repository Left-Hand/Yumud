#pragma once


namespace archive{
static constexpr size_t STORAGE_MAX_SIZE = 256;
static constexpr size_t HEADER_MAX_SIZE = 32;
static constexpr size_t CONTEXT_PLACE_OFFSET = HEADER_MAX_SIZE;
static constexpr size_t CONTEXT_MAX_SIZE = STORAGE_MAX_SIZE - CONTEXT_PLACE_OFFSET;

using Bin = std::array<uint8_t, STORAGE_MAX_SIZE>;

struct Header{
    HashCode hashcode;
    ReleaseInfo release_info;

    constexpr size_t size() const {
        return sizeof(*this);
    }

    std::span<const uint8_t> as_bytes() const {
        return std::span<const uint8_t>{
            reinterpret_cast<const uint8_t *>(this), size()};
    }
};

static_assert(sizeof(Header) <= HEADER_MAX_SIZE);

template<typename Iter>
static constexpr auto make_header(Iter iter) -> Header{ 
    const auto hashcode = hash(iter);
    const auto release_info = ReleaseInfo::from("Rstr1aN", ReleaseVersion{0,1}).unwrap();
    return Header{
        .hashcode = hashcode,
        .release_info = release_info
    };
}


struct Context{ 
};

static constexpr Bin make_bin(const Header & header, const Context & context){
    Bin bin;

    return bin;
};

[[maybe_unused]] static void static_test(){
    static constexpr auto header = []{
        auto payload_iter = RepeatIter<uint8_t>(0, 1);
        return make_header(payload_iter);
    }();

    static_assert(header.release_info.version == ReleaseVersion{0,1});
    static_assert(header.release_info.author.name()[0] == 'R');
    static_assert(header.release_info.author.name()[1] == 's');
    static_assert(header.release_info.author.name()[2] == 't');

    // static_assert(header.hashcode == 0);
}

}

namespace ymd{
template<>
struct serde::SerializeIterMaker<serde::RawBytes, archive::Header>{
    static constexpr auto make(const archive::Header & header){
        return serde::make_serialize_iter<serde::RawBytes>(
            std::make_tuple(header.hashcode, header.release_info));
    }
};
}