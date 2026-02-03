#pragma once

#include "core/stream/ostream.hpp"
#include "core/string/view/string_view.hpp"

namespace ymd{

class BufStream final:public OutputStream{
public:
    BufStream(std::span<char> pbuf):
        buf_(pbuf.data()),
        capacity_(pbuf.size()){;}

    size_t sendout(std::span<const uint8_t> pbuf){
        const size_t req_len = std::min(pbuf.size(), writable_capacity());

        std::copy(pbuf.data(), pbuf.data() + req_len, buf_);

        len_ += req_len;
        return req_len;

    }

    [[nodiscard]] constexpr size_t free_capacity() const {return capacity_ - len_;}

    [[nodiscard]] constexpr StringView inner_str() const {
        return StringView(buf_, len_);
    }
private:
    char * buf_;
    const size_t capacity_;
    size_t len_ = 0;

    size_t writable_capacity() const {
        return capacity_ - len_;
    }
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
