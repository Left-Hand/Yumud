#pragma once

#include "core/stream/ostream.hpp"
#include "core/string/string.hpp"

namespace ymd{

class BufStream:public OutputStream{
protected:
    char * buf_;
    const size_t max_len_;
    size_t len_ = 0;
public:
    template <typename T>
    requires std::ranges::contiguous_range<T> and (sizeof(T) == 1)
    BufStream(T& range):
        buf_(reinterpret_cast<char *>(std::ranges::data(range))),
        max_len_(std::ranges::size(range)){;}

    BufStream(char * buf, const size_t max_len = UINT32_MAX):
        buf_(buf),
        max_len_(max_len){;}

    void write(const char data);
    void write(const char * data_ptr, const size_t len);
    size_t pending() const {return 0;}


    operator String() const;
    operator StringView() const;
};

template<typename ... Args>
__inline size_t snprintf(char * buf, size_t len, Args && ... args){
    BufStream os(buf, len);
    (os << ... << std::forward<Args>(args));
    return os.pending();
}

template <typename T, typename ... Args>
requires std::ranges::contiguous_range<T> and (sizeof(T) == 1)
__inline size_t snprintf(T & range, Args && ... args){
    BufStream os(range);
    (os << ... << std::forward<Args>(args));
    return os.pending();
}

template<typename ... Args>
__inline size_t snprintf(OutputStream & os, Args && ... args){
    (os << ... << std::forward<Args>(args));
    return os.pending();
}
};
