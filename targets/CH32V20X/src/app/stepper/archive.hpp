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


#if 0
template<typename T>
struct Context{
    static_assert(std::is_copy_assignable_v<T>);

    ReleaseInfo release_info;
    T obj;

    template<HashAlgo S>
    friend Hasher<S> & operator << (Hasher<S> & hs, const Context & self){
        return hs << self.release_info;
    }
};

template<typename T>
struct Bin{
    HashCode hashcode;
    Context<T> context;

    constexpr HashCode calc_hash_of_context() const{
        return hash(context);
    } 

    constexpr bool is_verify_passed(){
        return calc_hash_of_context() == hashcode;
    } 
    constexpr const Context<T> * operator ->() const {
        return context;
    }

    constexpr size_t size() const {
        return sizeof(*this);
    }

    std::span<const uint8_t> as_bytes() const {
        return std::span<const uint8_t>{
            reinterpret_cast<const uint8_t *>(this), size()};
    }
};

#endif
}

namespace ymd{
template<>
struct serde::SerializeIterMaker<serde::RawBytes, archive::Header>{
    static constexpr auto make(const archive::Header & header){
        return serde::make_serialize_iter<serde::RawBytes>(
            std::make_tuple(header.hashcode, header.release_info));
    }
};


#if 0
class MotorArchiveSystem{
    MotorArchiveSystem(drivers::AT24CXX at24):
        at24_(at24){;}



    enum class State:uint8_t{
        Idle,
        Saving,
        Loading,
        Verifying
    };


    static_assert(sizeof(archive::Header) <= HEADER_MAX_SIZE);

    template<typename T>
    auto save(T & obj){
        const auto body_bytes = obj.as_bytes();
        const auto header = archive::generate_header(body_bytes);
        auto assign_header_and_obj_to_buf = [&]{
            std::copy(buf_.begin(), buf_.end(), header.as_bytes());
            std::copy(buf_.begin() + BODY_OFFSET, buf_.end(), obj.as_bytes());
        };

        assign_header_and_obj_to_buf();

        return at24_.store_bytes(Address(BODY_OFFSET), buf_);
    }

    template<typename T>
    auto load(T & obj){
        // const auto body_bytes = obj.as_bytes();
        // const Header header = {
        //     .hashcode = hash(body_bytes),
        //     .release_info = ReleaseInfo::from("Rstr1aN", {0,1})
        // };

        // auto assign_header_and_obj_to_buf = [&]{
        //     std::copy(buf_.begin(), buf_.end(), header.as_bytes());
        //     std::copy(buf_.begin() + BODY_OFFSET, buf_.end(), obj.as_bytes());
        // };

        // assign_header_and_obj_to_buf();

        // return at24_.store_bytes(Address(BODY_OFFSET), buf_);
    }

    auto poll(){
        if(not at24_.is_idle()){
            at24_.poll().examine();
        }
    }

    bool is_idle(){
        return at24_.is_idle();
    }

    Progress progress(){
        return {0,0};
    }
private:
    std::atomic<State> state_ = State::Idle;

    drivers::AT24CXX & at24_;
    std::array<uint8_t, STORAGE_MAX_SIZE> buf_;
};
#endif


#if 0

    // void save(std::span<const uint8_t>){
        // at24.load_bytes(0_addr, std::span(rdata)).examine();
        // while(not at24.is_idle()){
        //     at24.poll().examine();
        // }

        // DEBUG_PRINTLN(rdata);
        // const uint8_t data[] = {uint8_t(rdata[0]+1),2,3};
        // at24.store_bytes(0_addr, std::span(data)).examine();

        // while(not at24.is_idle()){
        //     at24.poll().examine();
        // }

        // DEBUG_PRINTLN("done", clock::micros() - begin_u);
        // while(true);
    // }


#endif
}