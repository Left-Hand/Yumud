#pragma once

#include <type_traits>
#include <concepts>


namespace ymd{


// 检测是否支持 next() 和 has_next()
template <typename Iter, typename = void>
struct is_next_based_iter : std::false_type {};

template <typename Iter>
struct is_next_based_iter<Iter, 
    std::void_t<
        decltype(std::declval<Iter>().next()),
        decltype(std::declval<Iter>().has_next())
    >> : std::true_type {};

template <typename Iter>
inline constexpr bool is_next_based_iter_v = is_next_based_iter<Iter>::value;

// 检测是否是标准迭代器（支持 ++ 和 *）
template <typename Iter, typename = void>
struct is_std_iter : std::false_type {};

template <typename Iter>
struct is_std_iter<Iter,
    std::void_t<
        decltype(++std::declval<Iter&>()),
        decltype(*std::declval<Iter&>())
    >> : std::true_type {};

template <typename Iter>
inline constexpr bool is_std_iter_v = is_std_iter<Iter>::value;

template <typename T, typename = void>
struct is_range : std::false_type {};

template <typename T>
struct is_range<T, 
    std::void_t<
        decltype(std::declval<T>().begin()),
        decltype(std::declval<T>().end())
    >> : std::true_type {};

template <typename T>
inline constexpr bool is_range_v = is_range<T>::value;

template <typename Iter>
struct StdRange {
};

// 特化 next()/has_next() 风格的迭代器
template <typename Iter>
requires(is_next_based_iter_v<std::decay_t<Iter>>)
struct StdRange<Iter> {
    struct Sentinel {};

    struct Iterator {
        using value_type = decltype(std::declval<Iter>().next());
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        explicit constexpr Iterator(Iter iter) : iter_(iter) {
            if (iter_.has_next()) {
                current_ = iter_.next();
            }
        }

        constexpr const value_type& operator*() const { return current_.value(); }
        constexpr value_type& operator*() { return current_.value(); }

        constexpr Iterator& operator++() {
            if (iter_.has_next()) {
                current_ = iter_.next();
            } else {
                current_.reset();
            }
            return *this;
        }

        constexpr Iterator operator++(int) {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        constexpr bool operator==(Sentinel) const {
            return !current_.has_value();
        }

    private:
        Iter iter_;
        std::optional<value_type> current_;
    };

    explicit constexpr StdRange(Iter iter) : iter_(iter) {}

    constexpr Iterator begin() const {
        return Iterator(iter_);
    }

    constexpr Sentinel end() const {
        return Sentinel{};
    }

private:
    Iter iter_;
};


// 推导指引：next()/has_next() 风格的迭代器
template <typename Iter>
requires(is_next_based_iter_v<std::decay_t<Iter>>)
StdRange(Iter) -> StdRange<std::decay_t<Iter>>;

#if 0
// 特化标准迭代器（支持 ++ 和 *）
template <typename Iter>
requires(is_std_iter_v<std::decay_t<Iter>>)
struct StdRange<Iter> {

    struct Iter{
        using value_type = typename std::iterator_traits<Iter>::value_type;
        using iterator_category = typename std::iterator_traits<Iter>::iterator_category;
        using difference_type = typename std::iterator_traits<Iter>::difference_type;
        using pointer = typename std::iterator_traits<Iter>::pointer;
        using reference = typename std::iterator_traits<Iter>::reference;

        explicit constexpr StdRange(Iter iter) : iter_(iter) {}

        constexpr reference operator*() const { return *iter_; }
        constexpr pointer operator->() const { return &(*iter_); }

        constexpr StdRange& operator++() {
            ++iter_;
            return *this;
        }

        constexpr StdRange operator++(int) {
            auto tmp = *this;
            ++iter_;
            return tmp;
        }

        constexpr bool operator==(const StdRange& other) const {
            return iter_ == other.iter_;
        }
    }


private:
    Iter iter_;
};


// 推导指引：标准迭代器
template <typename Iter>
requires(is_std_iter_v<std::decay_t<Iter>>)
StdRange(Iter) -> StdRange<std::decay_t<Iter>>;
#endif

// 特化范围类型（如 std::vector）
template <typename Range>
requires (is_range_v<std::decay_t<Range>>)
struct StdRange<Range> {
    using iterator_type = decltype(std::declval<Range>().begin());
    using sentinel_type = decltype(std::declval<Range>().end());

    struct Iterator {
        using value_type = typename std::iterator_traits<iterator_type>::value_type;
        using iterator_category = typename std::iterator_traits<iterator_type>::iterator_category;
        using difference_type = typename std::iterator_traits<iterator_type>::difference_type;
        using pointer = typename std::iterator_traits<iterator_type>::pointer;
        using reference = typename std::iterator_traits<iterator_type>::reference;

        constexpr Iterator(iterator_type iter, sentinel_type end) 
            : iter_(iter), end_(end) {}

        // 解引用
        constexpr reference operator*() const { return *iter_; }
        constexpr pointer operator->() const { return &(*iter_); }

        // 前置++
        constexpr Iterator& operator++() {
            ++iter_;
            return *this;
        }

        // 后置++
        constexpr Iterator operator++(int) {
            auto tmp = *this;
            ++iter_;
            return tmp;
        }

        // == 比较
        constexpr bool operator==(const Iterator& other) const {
            return iter_ == other.iter_;
        }

        // != 比较（C++20 前 range-for 需要）
        constexpr bool operator!=(const Iterator& other) const {
            return iter_ != other.iter_;
        }

        // 与哨位比较
        constexpr bool operator==(const sentinel_type& end) const {
            return iter_ == end;
        }

        constexpr bool operator!=(const sentinel_type& end) const {
            return iter_ != end;
        }

    private:
        iterator_type iter_;
        sentinel_type end_;
    };

    // 构造函数
    explicit constexpr StdRange(Range&& range) 
        : begin_(std::forward<Range>(range).begin()), 
            end_(std::forward<Range>(range).end()) {}

    // 提供 begin() 和 end()，支持 range-for
    constexpr Iterator begin() const { return Iterator(begin_, end_); }
    constexpr sentinel_type end() const { return end_; }

private:
    iterator_type begin_;
    sentinel_type end_;
};


// 推导指引：范围类型（如 std::vector）
template <typename Range>
requires (is_range_v<std::decay_t<Range>>)
StdRange(Range &&) -> StdRange<std::decay_t<Range>>;



}