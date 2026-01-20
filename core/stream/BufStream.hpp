#pragma once

#include "core/stream/ostream.hpp"
#include "core/string/view/string_view.hpp"

namespace ymd{

class BufStream final:public OutputStream{
public:
    BufStream(std::span<char> pbuf):
        buf_(pbuf.data()),
        max_len_(pbuf.size()){;}

    void sendout(std::span<const char> pbuf){
        if(pbuf.size() > writable_capacity()){
            while(true);
        }else{
            std::copy(pbuf.data(), pbuf.data() + pbuf.size(), buf_);
        }
    }

    constexpr size_t free_capacity() const {return max_len_ - len_;}

    constexpr operator StringView() const {
        return StringView(buf_, len_);
    }
private:
    char * buf_;
    const size_t max_len_;
    size_t len_ = 0;

    size_t writable_capacity() const {
        return max_len_ - len_;
    }
};

// template<typename ... Args>
// __inline size_t snprintf(char * buf, size_t len, Args && ... args){
//     BufStream os(buf, len);
//     (os << ... << std::forward<Args>(args));
//     return os.free_capacity();
// }

// template <typename T, typename ... Args>
// requires std::ranges::contiguous_range<T> and (sizeof(T) == 1)
// __inline size_t snprintf(T & range, Args && ... args){
//     BufStream os(range);
//     (os << ... << std::forward<Args>(args));
//     return os.free_capacity();
// }

// template<typename ... Args>
// __inline size_t snprintf(OutputStream & os, Args && ... args){
//     (os << ... << std::forward<Args>(args));
//     return os.free_capacity();
// }
};
