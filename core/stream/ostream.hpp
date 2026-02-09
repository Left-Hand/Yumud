#pragma once

#include "core/platform.hpp"

#include <bits/ios_base.h>
#include <iomanip>

#include <cstdint>
#include <string_view>
#include <type_traits>
#include <tuple>
#include <optional>
#include <utility>
#include <bitset>
#include <span>
#include <chrono>
#include <variant>

#include "core/stream/CharOpTraits.hpp"
#include "core/utils/stdrange.hpp"
#include "core/math/iq/fixed_t.hpp"


namespace std{

    template<class CharT, class Traits = std::char_traits<CharT>, class Allocator = std::allocator<CharT>>
    class basic_string;
    
    class source_location;

    struct strong_ording;
    struct weak_ording;
}

namespace ymd{

class String;
class StringView;
class MutStringView;
class StringStream;
class OutputStream;

template<typename T>
struct DeriveDebugDispatcher {};

template<typename T>
concept has_derive_debug_dispatcher = requires(OutputStream& os, const T& value) {
    DeriveDebugDispatcher<T>::call(os, value);
};


#define DEF_DERIVE_DEBUG(type)\
inline ::ymd::OutputStream& operator<<(::ymd::OutputStream& os,const type & value){\
    ::ymd::DeriveDebugDispatcher<type>::call(os, value);\
    return os;\
}\


#define DEF_FRIEND_DERIVE_DEBUG(type)\
friend ::ymd::OutputStream& operator<<(::ymd::OutputStream& os,const type & value){\
    ::ymd::DeriveDebugDispatcher<type>::call(os, value);\
    return os;\
}\


namespace details{
struct Splitter{};
struct Endl{};


template <typename T>
struct _need_insert_splitter_before{static constexpr bool value = true;};

template <>
struct _need_insert_splitter_before<std::ios_base& (*)(std::ios_base&)>
    :std::false_type{};

template<> struct _need_insert_splitter_before<std::_Setprecision>:std::false_type{};

template<> struct _need_insert_splitter_before<std::_Setbase>:std::false_type{};

template<> struct _need_insert_splitter_before<Splitter>:std::false_type{};

template<> struct _need_insert_splitter_before<std::_Swallow_assign>:std::false_type{};

template<typename T>
static constexpr bool need_insert_splitter_before_v =
    details::_need_insert_splitter_before<std::decay_t<T>>::value;

template<char c>
struct Brackets{
    static constexpr char chr = c;
};
}


class OutputStreamIntf{
public:
    [[nodiscard]] virtual size_t sendout(const std::span<const uint8_t>) = 0;
};

class [[nodiscard]] OutputStream:public OutputStreamIntf{
public:
    struct alignas(16) [[nodiscard]] Config{

        using Self = Config;

        struct Specifier{
            uint16_t boolalpha:1;
            uint16_t showpos:1;
            uint16_t showbase:1;
            uint16_t no_brackets:1;
            uint16_t no_space:1;
            uint16_t force_sync:1;
            uint16_t no_scoped:1;
            uint16_t no_fieldname:1;
        };

        // std::array<char, 4> splitter;
        char splitter[4];

        uint8_t splitter_len;
        uint8_t radix;
        uint8_t eps;
        uint8_t indent;

        char pandding;
        uint8_t pandding_len;

        union{
            uint16_t specifier_bits;
            Specifier specifier;
        };

        static constexpr Self from_default(){
            return Self{
                .splitter = {',', ' ', '\0', '\0'},
                .splitter_len = 2,
                .radix = 10,
                .eps = 4,
                .indent = 0,
                .pandding = ' ',
                .pandding_len = 0,
                .specifier_bits = 0
            };
        }
    };
private:

    using Splitter = details::Splitter;
    using Endl = details::Endl;

    template<char CHR>
    using Brackets = details::Brackets<CHR>;
public:
    OutputStream(){
        reconf(Config::from_default());
    }

    virtual ~OutputStream() = default;

    OutputStream(const OutputStream &) = delete;
    OutputStream(OutputStream &&) = delete;

    void write_byte(const uint8_t byte);

    void write_bytes(std::span<const uint8_t> bytes);

    OutputStream & set_splitter(const char * splitter){
        std::fill_n(config_.splitter, 4, 0);

        config_.splitter_len = 0;
        for(size_t i = 0; i < 4 && splitter[i] != '\0'; ++i) {
            config_.splitter[i] = splitter[i];
            config_.splitter_len++;
        }
        return *this;
    }

    OutputStream & set_splitter(const char splitter){
        std::fill_n(config_.splitter, 4, 0);
        config_.splitter[0] = splitter;
        config_.splitter_len = 1;
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

    [[nodiscard]] uint8_t indent() const{
        return config_.indent;
    }

    OutputStream & set_eps(const uint8_t eps){
        config_.eps = eps;
        return *this;
    }

    OutputStream & no_brackets(const Enable en){
        config_.specifier.no_brackets = bool(en == EN);
        return *this;
    }

    OutputStream & no_scoped(const Enable en){
        config_.specifier.no_scoped = bool(en == EN);
        return *this;
    }

    OutputStream & no_fieldname(const Enable en){
        config_.specifier.no_fieldname = bool(en == EN);
        return *this;
    }
    OutputStream & force_sync(const Enable en){
        config_.specifier.force_sync = bool(en == EN);
        return *this;
    }

    OutputStream & no_space(const Enable en){
        config_.specifier.no_space = bool(en == EN);
        return *this;
    }


    OutputStream & operator<<(const bool val);
    OutputStream & operator<<(const uint8_t val);

    __inline OutputStream & operator<<(const char chr){
        write_byte(chr); return *this;}

    OutputStream & operator<<(const wchar_t chr);

    // !warning, take care of your stupid null-terminated c-style string
    __inline OutputStream & operator<<(char * str){
        write_bytes(std::span<const uint8_t>(
            reinterpret_cast<const uint8_t *>(str),
            strlen(str))
        );
        return *this;
    }

    // !warning, take care of your stupid null-terminated c-style string
    __inline OutputStream & operator<<(const char* str){
        write_bytes(std::span<const uint8_t>(
            reinterpret_cast<const uint8_t *>(str),
            strlen(str))
        );
        return *this;
    }

    OutputStream & operator<<(const std::string & str);

    __inline OutputStream & operator<<(const std::string_view str){
        write_bytes(std::span<const uint8_t>(
            reinterpret_cast<const uint8_t *>(str.data()),
            str.length())
        );
        return *this;
    }

    OutputStream & operator<<(const String & str);
    OutputStream & operator<<(const StringView str);
    OutputStream & operator<<(const MutStringView str);
    __inline OutputStream & operator<<(const std::byte byte){return *this << (uint8_t(byte));}
    OutputStream & operator<<(const float val);
    OutputStream & operator<<(const double val);

    /// !不要试图移除这个函数的内联性 否则将会灾难性引入cxx11abi相关的大量无用函数 无异于直接引入iostream
    __always_inline OutputStream & operator<<(std::ostream& (*manipulator)(std::ostream&)){
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
    OutputStream & operator<<(const std::endian endian);
    OutputStream & operator<<(const std::strong_ordering & ordering);
    OutputStream & operator<<(const std::weak_ordering & ordering);

    template<typename T>
    OutputStream & operator<<(const std::chrono::duration<T, std::milli> ms){
        auto & self = *this;
        static constexpr std::array<uint8_t, 2> MS_BYTES = {'m', 's'}; 
        self << ms.count();
        self.write_bytes(std::span(MS_BYTES));
        return self;
    }

    template<typename T>
    OutputStream & operator<<(const std::chrono::duration<T, std::micro> us){
        auto & self = *this;
        static constexpr std::array<uint8_t, 2> US_BYTES = {'u', 's'}; 
        self << us.count();
        self.write_bytes(std::span(US_BYTES));
        return self;
    }

    template<typename T>
    OutputStream & operator<<(const std::chrono::duration<T, std::nano> ns){
        auto & self = *this;
        static constexpr std::array<uint8_t, 2> NS_BYTES = {'n', 's'}; 
        self << ns.count();
        self.write_bytes(std::span(NS_BYTES));
        return self;
    }

    OutputStream & operator<<(const Splitter){print_splt(); return *this;}

    template<char CHR>
    OutputStream & operator<<(const Brackets<CHR>){
        if(!config_.specifier.no_brackets) write_byte(CHR); 
        return *this;
    }

    OutputStream & operator<<(const std::source_location & loc){print_source_loc(loc); return *this;}

    template<typename T>
    OutputStream & operator<<(const std::optional<T> v){
        if(v.has_value()) return *this << v.value();
        else return *this << '/';
    }

    OutputStream & operator<<(const std::monostate);

    OutputStream & operator<<(const std::_Swallow_assign);
    OutputStream & operator<<(const std::_Setw);
    OutputStream & operator<<(const std::_Setfill<char> setfill);

    template<size_t N>
    OutputStream & operator<<(const std::bitset<N> bs){
        std::array<uint8_t, N> buf;
        for(size_t i = 0; i < N; ++i){
            buf[N - 1 - i] = (bs[i]) ? '1' : '0';
        }
        this->write_bytes(std::span(buf));
        return *this;
    }

    template<typename T>
    requires has_derive_debug_dispatcher<T>
    OutputStream & operator<<(T && val){
        DeriveDebugDispatcher<T>::call(*this, std::forward<T>(val));
        return *this;
    }

    // template<typename T>
    // OutputStream & operator<<(T && val){
    //     static_assert(magic::false_v<T>, "unsupported type");
    //     return *this;
    // }

    //#region print integer
private:
    void print_u32(const uint32_t i_val);
    void print_i32(const int32_t i_val);
    void print_u64(const uint64_t i_val);
    void print_i64(const int64_t i_val);



    void print_iq16(const math::fixed_t<16, int32_t> q_val);
public:

    template<size_t Q, typename D>
    OutputStream & operator<<(const math::fixed_t<Q, D> & q_val){
        print_iq16(math::fixed_t<16, int32_t>(q_val));
        return *this;
    }

    template<typename T>
    requires (std::is_integral_v<T> and (sizeof(T) <= 8))
    OutputStream & operator<<(const T val){
        if constexpr(sizeof(T) <= 4){
            if constexpr (std::is_signed_v<T>){
                print_i32(int32_t(val));
            }else{
                print_u32(uint32_t(val));
            }
        }else if constexpr(sizeof(T) <= 8){
            if constexpr (std::is_signed_v<T>){
                print_i64(int64_t(val));
            }else{
                print_u64(uint64_t(val));
            }
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
    void print_span(const T * begin, const size_t size){
        *this << this->brackets<'['>();
        if(size > 0){
            for(size_t i = 0; i < size_t(size - 1); ++i) *this << begin[i] << ',';
            *this << begin[size - 1];
        }else{
            *this << '\\';
        }
        *this << this->brackets<']'>();
    }

    template <typename... Args>
    void print_tuple(const std::tuple<Args...> & t){
        using TupleType = std::tuple<Args...>;
        constexpr size_t TUP_SIZE = std::tuple_size_v<TupleType>;
        
        *this << brackets<'('>();
        
        [&]<size_t... Is>(std::index_sequence<Is...>) {
            ((Is > 0 ? (*this << splitter() << std::get<Is>(t)) : (*this << std::get<Is>(t))), ...);
        }(std::make_index_sequence<TUP_SIZE>{});
        
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


    template <std::ranges::range R>
    OutputStream& operator<<(R&& range) {  // 改为万能引用
        *this << brackets<'['>();

        auto it = std::ranges::begin(range);
        auto end = std::ranges::end(range);

        if (it != end) {
            *this << *it;
            ++it;

            for (; it != end; ++it) {
                *this << ',' << *it;
            }
        } else {
            *this << '\\';
        }

        *this << brackets<']'>();
        return *this;
    }

    template<typename T>
    requires is_next_based_iter_v<T>
    OutputStream& operator<<(const T& iter) {
        auto & self = *this;
        self << brackets<'['>();

        bool is_first = true;
        auto temp_iter = iter; // 创建副本以避免修改原迭代器

        while (temp_iter.has_next()) {
            if (!is_first) {
                self << ',';
            }
            self << temp_iter.next();
            is_first = false;
        }

        // 如果没有任何元素，可以输出特定标记（如原代码中的'\'）
        if (is_first) {
            self << '\\';
        }

        self << self.template brackets<']'>();
        return self;
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

        if constexpr(false == details::need_insert_splitter_before_v<First>){
            return prints(std::forward<Args>(args)...);
        }else if constexpr (sizeof...(args)) {
            (print_given_splt_then_entity(' ', std::forward<Args>(args)), ...);
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
        if constexpr(false == details::need_insert_splitter_before_v<First>){
            return printt(std::forward<Args>(args)...);
        }else if constexpr (sizeof...(args)) {
            (print_given_splt_then_entity('\t', std::forward<Args>(args)), ...);
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
        if constexpr(false == details::need_insert_splitter_before_v<First>){
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

    struct [[nodiscard]] FieldName final{

        __inline explicit FieldName(OutputStream & os, const std::string_view name):
            os_(os){
            if(not os.config().specifier.no_fieldname)
                os << name << ':';
        }

        FieldName(const FieldName &) = delete;
        FieldName(FieldName &&) = delete;

        template<typename ... Args>
        __inline FieldName & operator()(Args&& ... args){
            if constexpr (sizeof...(args)) {
                ((os_ << std::forward<Args>(args)), ...);
            }
            return *this;
        }

        __inline friend OutputStream & operator<<(OutputStream & os, const FieldName & self){
            return os;
        }
    private:
        OutputStream & os_;
    };

    friend struct FieldName;


    struct [[nodiscard]] ScopedInfo{
        __inline explicit ScopedInfo(OutputStream & os, const std::string_view name):
            os_(os){
            if(not os.config().specifier.no_scoped){
                os << name << ':';
                os << os.brackets<'{'>();
            }
        }

        ScopedInfo(const ScopedInfo &) = delete;
        ScopedInfo(ScopedInfo &&) = delete;

        template<typename ... Args>
        __inline ScopedInfo & operator()(Args&& ... args){
            if constexpr (sizeof...(args)) {
                ((os_ << std::forward<Args>(args)), ...);
            }
            if(not os_.config().specifier.no_scoped)
                os_ << os_.brackets<'}'>();
            return *this;
        }

        friend OutputStream & operator<<(OutputStream & os, const ScopedInfo & info){
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
        return *this;
    }

    const Config & config() const {return config_;}

    class [[nodiscard]] ConfigGuard{
        OutputStream & os_;
        const Config config_;
    public:
        ConfigGuard(OutputStream & os) : os_(os), config_(os.config()){}

        constexpr const Config & config() const {return config_;}
        ~ConfigGuard(){
            os_.reconf(config_);
        }
    };

    [[nodiscard]] ConfigGuard create_guard(){
        return ConfigGuard(*this);
    }

private:
    Config config_;

    __fast_inline void print_splt(){
        write_bytes(std::span<const uint8_t>(reinterpret_cast<const uint8_t *>(config_.splitter), config_.splitter_len));
    }

    template<typename T>
    __fast_inline void print_splt_then_entity(T && any){
        if constexpr(details::need_insert_splitter_before_v<T>){
            print_splt();
        }
        *this << std::forward<T>(any);
    }

    template<typename T>
    __fast_inline void print_given_splt_then_entity(const char splt, T && any){
        if constexpr(details::need_insert_splitter_before_v<T>){
            write_byte(splt);
        }
        *this << std::forward<T>(any);
    }

    __fast_inline void print_end(){
        flush();
        // if((config_.specifier.force_sync)) [[unlikely]] {
        //     block_util_least_free_capacity(1u);
        // }
    }

    __fast_inline void print_indent(){
        if((config_.indent == 0)) [[likely]]
            return;
        for(size_t i = 0; i < config_.indent; i++){
            write_byte('\t');
        }
    }

    __fast_inline void print_endl(){
        static constexpr const char * enter_str = "\r\n";
        static constexpr size_t enter_str_len = 2;

        write_bytes(std::span<const uint8_t>(
            reinterpret_cast<const uint8_t *>(enter_str),
            enter_str_len
        ));
        print_end();
    }



    void print_source_loc(const std::source_location & loc);

    #ifndef OSTREAM_BUF_SIZE
    static constexpr size_t OSTREAM_BUF_SIZE = 60;
    #endif

    struct Buf{
        std::array<uint8_t, OSTREAM_BUF_SIZE> buf;
        size_t size = 0;


        // 用于压入数据，当数据溢满时发送数据包
        template<typename Fn>
        __fast_inline void push_bytes(const std::span<const uint8_t> bytes, Fn&& fn) {
            size_t offset = 0;
            while (offset < bytes.size()) {
                size_t free_cap = OSTREAM_BUF_SIZE - size;
                size_t clone_size = std::min(free_cap, bytes.size() - offset);

                std::copy(
                    bytes.data() + offset,
                    bytes.data() + offset + clone_size,
                    buf.data() + size
                );

                size += clone_size;
                offset += clone_size;

                if (size >= OSTREAM_BUF_SIZE) {
                    std::forward<Fn>(fn)(std::span<const uint8_t>(buf.data(), size));  // 发送缓冲区数据
                    clear();  // 发送后重置缓冲区
                }
            }
        }

        // 强制刷新缓冲区（发送剩余数据）
        template<typename Fn>
        __fast_inline void flush(Fn&& fn) {
            if (size > 0) {
                std::forward<Fn>(fn)(std::span<const uint8_t>(buf.data(), size));  // 发送缓冲区数据
                clear();  // 发送后重置缓冲区
            }
        }

        // 清空缓冲区（不发送数据）
        __fast_inline void clear() {
            size = 0;
        }
    };

    // Buf buf_;
};


class OutputStreamByRoute final: public OutputStream{
private:
    using Traits = WriteCharFacade;
    using Route = pro::proxy<Traits>;
    Route p_route_;

    size_t sendout(const std::span<const uint8_t> bytes);
public:
    OutputStreamByRoute(){;}

    OutputStreamByRoute(Route && route):
        p_route_(std::move(route)){;}


    [[nodiscard]] size_t free_capacity() const {
        if(p_route_ == nullptr) [[unlikely]]
            __builtin_trap();
        return p_route_->free_capacity();
    }

    void retarget(Route && p_route){
        p_route_ = std::move(p_route);
    }

    Route & route() {
        if(p_route_ == nullptr) [[unlikely]]
            __builtin_trap();
        return p_route_;
    }
};

}
