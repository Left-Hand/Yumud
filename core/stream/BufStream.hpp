#pragma once

#include "core/stream/ostream.hpp"
#include "core/string/view/string_view.hpp"

namespace ymd{

class BufStream final:public OutputStream{
public:
    explicit BufStream(std::span<uint8_t> pbuf):
        buf_(pbuf.data()),
        capacity_(pbuf.size()){
            reset();
        }

    void reset(){
        len_ = 0;
    }

    size_t sendout(std::span<const uint8_t> pbuf){
        const size_t req_len = std::min(pbuf.size(), free_capacity());

        std::copy(pbuf.data(), pbuf.data() + req_len, buf_ + len_);

        len_ += req_len;
        return req_len;

    }

    [[nodiscard]] constexpr size_t free_capacity() const {return capacity_ - len_;}

    [[nodiscard]] StringView collected_str() const {
        return StringView(reinterpret_cast<const char *>(buf_), len_);
    }
private:
    uint8_t * buf_;
    const size_t capacity_;
    size_t len_;

};


#if 0
template<typename ... Args>
__inline size_t fmt_args(StringView str, Args && ... args){
    BufStream os(str, len);
    (os << ... << std::forward<Args>(args));
    return os.free_capacity();
}

template <typename T, typename ... Args>
requires std::ranges::contiguous_range<T> and (sizeof(T) == 1)
__inline size_t fmt_args(T & range, Args && ... args){
    BufStream os(range);
    (os << ... << std::forward<Args>(args));
    return os.free_capacity();
}

template<typename ... Args>
__inline size_t fmt_args(OutputStream & os, Args && ... args){
    (os << ... << std::forward<Args>(args));
    return os.free_capacity();
}
#endif

};
