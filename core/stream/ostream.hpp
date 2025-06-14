#pragma once


#include <ostream>
#include <span>
#include <ranges>
#include <cstring>
#include <chrono>

#include "stream_base.hpp"
#include "core/stream/CharOpTraits.hpp"
#include "core/buffer/ringbuf/Fifo_t.hpp"


namespace std{
    class source_location;
}

namespace ymd{

class String;
class StringView;
class StringStream;

template<typename T>
struct DeriveDebugDispatcher {};

template<typename T>
concept has_derive_debug_dispatcher = requires(OutputStream& os, const T& value) {
    DeriveDebugDispatcher<T>::call(os, value);
};


#define DERIVE_DEBUG(type)\
inline ::ymd::OutputStream& operator<<(::ymd::OutputStream& os,const type & value){\
    ::ymd::DeriveDebugDispatcher<type>::call(os, value);\
    return os;\
}\


#define FRIEND_DERIVE_DEBUG(type)\
friend ::ymd::OutputStream& operator<<(::ymd::OutputStream& os,const type & value){\
    ::ymd::DeriveDebugDispatcher<type>::call(os, value);\
    return os;\
}\


template<size_t Q>
struct iq_t;

namespace details{
struct Splitter{};
struct Endl{};


template <typename T>
struct _need_display{static constexpr bool value = true;};

template <>
struct _need_display<std::ios_base& (*)(std::ios_base&)>
    {static constexpr bool value = false;};

template<> struct _need_display<std::_Setprecision>{static constexpr bool value = false;};

template<> struct _need_display<std::_Setbase>{static constexpr bool value = false;};

template<> struct _need_display<Splitter>{static constexpr bool value = false;};

template<typename T>
static constexpr bool need_display_v = details::_need_display<std::decay_t<T>>::value;

template<char c>
struct Brackets{
    static constexpr char chr = c;
};

// template<typename T>
// struct _is_stringlike:  std::false_type{};

// template<> struct _is_stringlike<std::string>{static constexpr bool value = true;};
// template<> struct _is_stringlike<std::string_view>{static constexpr bool value = true;};

// template<> struct _is_stringlike<String>{static constexpr bool value = true;};
// template<> struct _is_stringlike<StringView>{static constexpr bool value = true;};

// template<typename T>
// inline constexpr bool is_stringlike_v = 
//     _is_stringlike<std::decay_t<T>>::value;


// template<typename T>
// struct _inhibit_display_asrange{static constexpr bool value = false;};

// template<typename T>
// requires is_stringlike_v<T>
// struct _inhibit_display_asrange<T>{static constexpr bool value = true;};

// template<size_t N>
// struct _inhibit_display_asrange<char[N]>{static constexpr bool value = true;};

// template<size_t N>
// struct _inhibit_display_asrange<std::bitset<N>>{static constexpr bool value = true;};


// template<typename T>
// inline constexpr bool inhibit_display_asrange_v = _inhibit_display_asrange<std::decay_t<T>>::value;


// // 概念：检测是否有 base() 方法
// template <typename R>
// concept has_base = requires(R r) {
//     { r.base() } -> std::ranges::range;
// };

// // 主模板
// template <typename R>
// struct underlying_range {
//     using type = R;
// };

// // 有 base() 方法的特化
// template <has_base R>
// struct underlying_range<R> : underlying_range<decltype(std::declval<R>().base())> {};

// template <typename R>
// using underlying_range_t = typename underlying_range<R>::type;

// // 检查是否是连续容器或基于连续容器的视图
// template <typename R>
// static inline constexpr bool 
// is_or_derived_from_contiguous_v = 
//     std::ranges::contiguous_range<R> ||
//     std::ranges::contiguous_range<underlying_range_t<R>>;

// template<typename T>
// static inline constexpr bool
// false_v = false; 
}




class OutputStreamIntf{
public:
    virtual size_t pending() const = 0;

    virtual void sendout(const std::span<const char>) = 0;
};

class OutputStream:public OutputStreamIntf{
public:
    struct Config{
        char splitter[4];

        uint8_t radix:5;
        uint8_t eps:3;
        uint8_t indent;
        union{
            uint16_t flags;
            struct{
                uint16_t boolalpha:1;
                uint16_t showpos:1;
                uint16_t showbase:1;
                uint16_t no_brackets:1;
                uint16_t no_space:1;
                uint16_t force_sync:1;
                uint16_t no_scoped:1;
                uint16_t no_fieldname:1;
            };
        };
    };
private:

    using Splitter = details::Splitter;
    using Endl = details::Endl;
    
    template<char c>
    using Brackets = details::Brackets<c>;


    uint8_t sp_len;

    scexpr Config DEFAULT_CONFIG = {
        .splitter = ", ",
        .radix = 10,
        .eps = 3,
        .indent = 0,
        .flags = 0,
    };

    Config config_;

    __fast_inline void print_splt(){
        write(config_.splitter, sp_len);
    }

    template<typename T>
    __fast_inline void print_splt_then_entity(T && any){
        if constexpr(details::need_display_v<T>){
            print_splt();
        }
        *this << std::forward<T>(any);
    }

    template<typename T>
    __fast_inline void print_splt_then_entity(const char splt, T && any){
        if constexpr(details::need_display_v<T>){
            write(splt);
        }
        *this << std::forward<T>(any);
    }

    __fast_inline void print_end(){
        flush();
        if(unlikely(config_.force_sync)){
            blocking_until_less_than(0);
        }
    }

    __fast_inline void blocking_until_less_than(size_t n){
        while(pending() > n) __nopn(1);
    }

    __fast_inline void print_indent(){
        if(likely(config_.indent == 0)) return;
        for(size_t i = 0; i < config_.indent; i++){
            write('\t');
        }
    }

    __fast_inline void print_endl(){
        scexpr const char * enter_str = "\r\n";
        scexpr size_t enter_str_len = 2;
        
        write(enter_str, enter_str_len);
        print_end();
    }

    scexpr const char * get_basealpha(const size_t _radix){
        switch(_radix){
            default:
            case 10:
                return "";
            case 2:
                return "0b";
            case 8:
                return "0";
            case 16:
                return "0x";
        }
    }

    int transform_char(const char chr) const;

    void write_checked(const char data){
        const auto res = transform_char(data);
        if(res >= 0) write(res);
    }

    void write_checked(const char * pbuf, const size_t len);

    void print_source_loc(const std::source_location & loc);

    #ifndef OSTREAM_BUF_SIZE
    static constexpr size_t OSTREAM_BUF_SIZE = 64;
    #endif

    struct Buf{
        char buf[OSTREAM_BUF_SIZE];
        uint8_t size = 0;
        
        
        // 用于压入数据，当数据溢满时发送数据包
        template<typename Fn>
        __fast_inline void push(const std::span<const char> pbuf, Fn&& fn) {
            size_t offset = 0;
            while (offset < pbuf.size()) {
                size_t available = OSTREAM_BUF_SIZE - size;
                size_t copy_size = std::min(available, pbuf.size() - offset);

                std::memcpy(buf + size, pbuf.data() + offset, copy_size);
                size += static_cast<uint8_t>(copy_size);
                offset += copy_size;

                if (size == OSTREAM_BUF_SIZE) {
                    fn(std::span<const char>(buf, OSTREAM_BUF_SIZE));  // 发送缓冲区数据
                    clear();  // 发送后重置缓冲区
                }
            }
        }

        // 用于压入数据，当数据溢满时发送数据包
        template<typename Fn>
        __fast_inline void push(char data, Fn&& fn) {
            buf[size++] = data;
            if (size == OSTREAM_BUF_SIZE) {
                fn(std::span<const char>(buf, OSTREAM_BUF_SIZE));  // 发送缓冲区数据
                clear();  // 发送后重置缓冲区
            }
        }

        // 强制刷新缓冲区（发送剩余数据）
        template<typename Fn>
        __fast_inline void flush(Fn&& fn) {
            if (size > 0) {
                fn(std::span<const char>(buf, size));  // 发送缓冲区数据
                clear();  // 发送后重置缓冲区
            }
        }

        // 清空缓冲区（不发送数据）
        __fast_inline void clear() {
            size = 0;
        }
    };

    Buf buf_;
public:
    OutputStream(){
        reconf(DEFAULT_CONFIG);
    }

    virtual ~OutputStream() = default;

    OutputStream(const OutputStream &) = delete;
    OutputStream(OutputStream &&) = delete;
    
    void write(const char data) {
        buf_.push(data, 
        [this](const std::span<const char> pbuf){
            this->blocking_until_less_than(OSTREAM_BUF_SIZE - pbuf.size());
            this->sendout(pbuf);
        });
    }
    void write(const char * pbuf, const size_t len){
        buf_.push(std::span<const char>(pbuf, len),  
        [this](const std::span<const char> pbuf){
            this->blocking_until_less_than(OSTREAM_BUF_SIZE - pbuf.size());
            this->sendout(pbuf);
        });
	}

    OutputStream & set_splitter(const char * splitter){
        strcpy(config_.splitter, splitter);
        sp_len = strlen(splitter);
        return *this;
    }
    
    OutputStream & set_splitter(const char splitter){
        config_.splitter[0] = splitter;
        config_.splitter[1] = 0;
        sp_len = 1;
        return *this;
    }

    OutputStream & set_radix(const uint8_t radix){
        config_.radix = radix;
        return *this;
    }

    OutputStream & set_indent(const uint8_t indent){
        config_.indent = indent;
        return *this;
    }

    uint8_t indent() const{
        return config_.indent;
    }

    OutputStream & set_eps(const uint8_t eps){
        config_.eps = eps;
        return *this;
    }

    OutputStream & no_brackets(const Enable en = EN){
        config_.no_brackets = bool(en == EN);
        return *this;
    }

    OutputStream & no_scoped(const Enable en = EN){ 
        config_.no_scoped = bool(en == EN);
        return *this;
    }

    OutputStream & no_fieldname(const Enable en = EN){ 
        config_.no_fieldname = bool(en == EN);
        return *this;
    }
    OutputStream & force_sync(const Enable en = EN){
        config_.force_sync = bool(en == EN);
        return *this;
    }

    OutputStream & no_space(const Enable en = EN){
        config_.no_space = bool(en == EN);
        return *this;
    }


    OutputStream & operator<<(const bool val);

    __inline OutputStream & operator<<(const char chr){write_checked(chr); return *this;}
    __inline OutputStream & operator<<(const wchar_t chr){write_checked(chr); return *this;}
    __inline OutputStream & operator<<(char * str){if(str) write_checked(str, strlen(str)); return *this;}
    __inline OutputStream & operator<<(const char* str){if(str) write_checked(str, strlen(str)); return *this;}
    __inline OutputStream & operator<<(const std::string & str){write_checked(str.c_str(),str.length()); return *this;}
    __inline OutputStream & operator<<(const std::string_view str){write_checked(str.data(),str.length()); return *this;}
    OutputStream & operator<<(const String & str);
    OutputStream & operator<<(const StringView & str);
    __inline OutputStream & operator<<(const std::byte chr){return *this << (uint8_t(chr));}
    OutputStream & operator<<(const float val);
    OutputStream & operator<<(const double val);
    OutputStream & operator<<(std::ostream& (*manipulator)(std::ostream&)) {
        if (manipulator == static_cast<std::ostream& (*)(std::ostream&)>(std::endl)) {
            *this << "\r\n";
            this->flush();
        }
        return *this;
    }

    OutputStream & operator<<(std::ios_base& (*func)(std::ios_base&));
    OutputStream & operator<<(const std::_Setprecision n){config_.eps = n._M_n; return *this;}
    OutputStream & operator<<(const std::_Setbase n){config_.radix = n._M_base; return *this;}
    OutputStream & operator<<(const Endl){this->print_endl(); return *this;}
    
    OutputStream & operator<<(const std::nullopt_t){return *this << '/';}


    template<typename T>
    OutputStream & operator<<(const std::chrono::duration<T, std::milli> ms){
        return *this << ms.count() << "ms";}

    template<typename T>
    OutputStream & operator<<(const std::chrono::duration<T, std::micro> us){
        return *this << us.count() << "us";}

    template<typename T>
    OutputStream & operator<<(const std::chrono::duration<T, std::nano> ns){
        return *this << ns.count() << "ns";}
    OutputStream & operator<<(const Splitter){print_splt(); return *this;}

    template<char chr>
    OutputStream & operator<<(const Brackets<chr>){if(!config_.no_brackets){write(chr);} return *this;}
    OutputStream & operator<<(const std::source_location & loc){print_source_loc(loc); return *this;}

    template<typename T>
    OutputStream & operator<<(const std::optional<T> v){
        if(v.has_value()) return *this << v.value();
        else return *this << '/';
    }

    template<size_t N>
    OutputStream & operator<<(const std::bitset<N> bs){
        char str[N + 1];
        for(size_t i = 0; i < N; ++i){
            str[N - 1 - i] = (bs[i]) ? '1' : '0';
        }
        str[N] = '\0';
        return *this << str;
    }

    template<typename T>
    requires has_derive_debug_dispatcher<T>
    OutputStream & operator<<(T && val){
        DeriveDebugDispatcher<T>::call(*this, std::forward<T>(val));
        return *this;
    }

    //#region print integer
private:
    void print_int(const int val);
    void print_u64(const uint64_t val);
    void print_i64(const int64_t val);
    
    __inline void print_numeric(const char * str, const size_t len, const bool pos){
        if(config_.showpos and pos) *this << '+';
        this->write(str, len);
    }
public:

    template<size_t Q>
    OutputStream & operator<<(const iq_t<Q> & val){
        char str[12] = {0};
        const auto len = qtoa<Q>(val, str, this->eps());
        print_numeric(str, len, val >= 0);
        return *this;
    }

    template<typename T>
    requires std::is_integral_v<T>
    OutputStream & operator<<(const T val){
        if constexpr(sizeof(T) <= 4){
            print_int(int(val));
        }else if constexpr (std::is_signed_v<T>){
            print_i64(int64_t(val));
        }else{
            print_u64(uint64_t(val));
        }
        return *this;
    }

    //#endregion
private:
    //#region print vased containers

    // Generic range printer with custom separator
    template <typename T, typename SeparatorFunc>
    void print_range_impl(const T& range, SeparatorFunc&& sep_func) {
        auto & self = *this;
        auto begin = std::ranges::begin(range);
        auto end = std::ranges::end(range);
        
        self << brackets<'['>();
        if (begin != end) {
            // Print first element
            self << *begin;
            ++begin;
            
            // Print remaining elements with separator
            for (; begin != end; ++begin) {
                self << sep_func();
                self << *begin;
            }
        } else {
            self << '\\';  // Empty range marker
        }
        self << brackets<']'>();
    }

    // Print contiguous range with comma separator
    template <typename T>
    requires std::ranges::contiguous_range<T>
    void print_contiguous_range(const T& range) {
        print_range_impl(range, [this]() {
            return ',';
        });
    }

    // Print linked range with arrow separator
    template <typename T>
    requires std::ranges::forward_range<T>
    void print_chained_range(const T& range) {
        print_range_impl(range, [this]() {
            return "=>";
        });
    }

    template<typename T>
    void print_span(const T * _begin, const size_t _size){
        *this << this->brackets<'['>();
        if(_size > 0){
            for(size_t i = 0; i < size_t(_size - 1); ++i) *this << _begin[i] << ',';
            *this << _begin[_size - 1];
        }else{
            *this << '\\';
        }
        *this << this->brackets<']'>();
    }

    template <typename... Args>
    void print_tuple(const std::tuple<Args...> & t){
        using TupleType = std::tuple<Args...>;
        constexpr size_t tupleSize = std::tuple_size<TupleType>::value;
        *this << brackets<'('>();
        std::apply(
            [&](const auto&... args) {
                ((tupleSize > 1 && &args != &std::get<tupleSize - 1>(t) 
                    ? (*this << args << ',') : (*this << args)), ...);
            },
            t
        );
        *this << brackets<')'>();
    }
public:
    // template <std::ranges::range R>
    // requires ((not details::inhibit_display_asrange_v<R>))
    // OutputStream & operator<<(R && range) {
    //     if constexpr(details::is_or_derived_from_contiguous_v<R>){
    //         print_contiguous_range(std::forward<R>(range));
    //     }else if constexpr(std::ranges::forward_range<R>){
    //         print_chained_range(std::forward<R>(range));
    //     }else{
    //         static_assert(details::false_v<R>, 
    //             "ostream does not support this range type");
    //     }
    //     return *this;
    // }
    template <typename T>
    requires std::ranges::contiguous_range<T>
    OutputStream & operator<<(const T& range) {
        print_span(std::ranges::data(range), std::ranges::size(range));
        return *this;
    }
    //#endregion


    template <typename... Args>
    OutputStream & operator<<(const std::tuple<Args...>& t) {
        print_tuple(t);
        return *this;
    }

    template <typename ... Args>
    OutputStream & print(Args&&... args){
        print_indent();
        if constexpr (sizeof...(args)) {
            ((*this << std::forward<Args>(args)), ...);
        }
        return *this;
    }

    
    template <typename First, typename ... Args>
    OutputStream & prints(First && first, Args&&... args){
        print_indent();
        *this << std::forward<First>(first);

        if constexpr(false == details::need_display_v<First>){
            return prints(std::forward<Args>(args)...);
        }else if constexpr (sizeof...(args)) {
            (print_splt_then_entity(' ', std::forward<Args>(args)), ...);
        }
        return prints();
    }

    OutputStream & prints(){
        print_endl();
        return *this;
    }

    template <typename First, typename ... Args>
    OutputStream & printt(First && first, Args&&... args){
        print_indent();
        *this << std::forward<First>(first);
        if constexpr(false == details::need_display_v<First>){
            return printt(std::forward<Args>(args)...);
        }else if constexpr (sizeof...(args)) {
            (print_splt_then_entity('\t', std::forward<Args>(args)), ...);
        }
        return printt();
    }

    OutputStream & printt(){
        print_endl();
        return *this;
    }

    template <typename First, typename ... Args>
    OutputStream & println(First && first, Args&&... args){
        print_indent();
        *this << std::forward<First>(first);
        if constexpr(false == details::need_display_v<First>){
            return println(std::forward<Args>(args)...);
        }else if constexpr (sizeof...(args)) {
            (print_splt_then_entity(std::forward<Args>(args)), ...);
        }
        return println();
    }

    OutputStream & println(){
        print_endl();
        return *this;
    }

    [[nodiscard]] __attribute__((const)) constexpr auto eps() const {return config_.eps;}
    [[nodiscard]] __attribute__((const)) constexpr auto radix() const {return config_.radix;}


    [[nodiscard]] __attribute__((const)) 
    static constexpr Endl endl(){return {};}

    [[nodiscard]] __attribute__((const)) 
    static constexpr Splitter splitter(){return {};}

    template<char chr>
    [[nodiscard]] __attribute__((const)) 
    static constexpr Brackets<chr> brackets(){return {};}

    struct FieldName final{

        explicit FieldName(OutputStream & os, const std::string_view name):
            os_(os){
            if(not os.config().no_fieldname)
                os << name << ':';
        }

        FieldName & operator()(OutputStream & os){
            return *this;
        }

        __fast_inline friend OutputStream & operator<<(OutputStream & os, const FieldName & fn){ 
            return os;
        }
    private:
        OutputStream & os_;
    };

    friend struct FieldName;


    struct ScopedInfo{
        explicit ScopedInfo(OutputStream & os, const std::string_view name):
            os_(os){
            if(not os.config().no_scoped){
                os << name << ':';
                os << os.brackets<'{'>();
            }
        }

        ScopedInfo(const ScopedInfo &) = delete;
        ScopedInfo(ScopedInfo &&) = delete;

        ScopedInfo & operator()(OutputStream & os){
            return *this;
        }

        friend OutputStream & operator<<(OutputStream & os, const ScopedInfo & info){ 
            if(not os.config().no_scoped) 
                os << os.brackets<'}'>();
            return os;
        }
    private:
        OutputStream & os_;
    };


    [[nodiscard]] ScopedInfo scoped(const std::string_view name){
        return ScopedInfo(*this, name);}

    [[nodiscard]] FieldName field(const std::string_view name){
        return FieldName(*this, name);}


    OutputStream & flush();

    OutputStream & reconf(const Config config){
        config_ = config;
        sp_len = strlen(config_.splitter);
        return *this;
    }

    Config config() const {return config_;}

    class __Guard{
        OutputStream & os_;
        const Config config_;
    public:
        __Guard(OutputStream & os) : os_(os), config_(os.config()){}

        ~__Guard(){
            os_.reconf(config_);
        }
    };

    [[nodiscard]] __Guard create_guard(){
        return __Guard(*this);
    }
};


class OutputStreamByRoute final: public OutputStream{
private:
    using Traits = WriteCharTraits;
    using Route = pro::proxy<Traits>;
    Route p_route_;

    void sendout(const std::span<const char> pbuf);
public:
    OutputStreamByRoute(){;}

    OutputStreamByRoute(Route && route):    
        p_route_(std::move(route)){;}


    size_t pending() const {
        if(unlikely(!p_route_)) while(true);
        return p_route_->pending();
    }

    void retarget(Route && p_route){
        p_route_ = std::move(p_route);
    }

    Route & route() {
        if(unlikely(!p_route_)) while(true);
        return p_route_;
    }
};

}