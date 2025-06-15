#pragma once

#include <ranges>

namespace ymd::fp{
// https://www.bluepuni.com/archives/zip-for-cpp20/
template <std::ranges::input_range ...Views>
class Zip : public std::ranges::view_interface<Zip<Views...>> {
public:
    struct iterator;
    struct sentinel;

public:
    Zip() = default;
    // Views are cheap to copy, but owning views cannot be done. (= delete)
    constexpr Zip(Views ...vs) noexcept: _views(std::move(vs)...) {}
    constexpr auto begin() {
        return std::apply([&](Views &...views) { return iterator(views...); }, _views);
    }
    constexpr auto end() requires (std::ranges::random_access_range<Views> && ...) {
        return sentinel{this};
    }
    constexpr auto size() const requires (std::ranges::sized_range<Views> && ...) {
        return std::apply([&](auto &&...views)
            { return std::min({std::ranges::size(views)...}); }, _views);
    }

private:
    std::tuple<Views...> _views;
};

template <std::ranges::input_range ...Views>
struct Zip<Views...>::iterator {
    friend struct sentinel;
    // TODO: flexible iterator_concepts.
    using iterator_concept = std::random_access_iterator_tag;
    using iterator_category = std::input_iterator_tag;
    using value_type = std::tuple<std::ranges::range_value_t<Views>...>;
    using difference_type = std::common_type_t<std::ranges::range_difference_t<Views>...>;

    iterator() = default;
    constexpr iterator(Views &...views): _currents{std::ranges::begin(views)...} {}

    constexpr auto operator*() const {
        return std::apply([&](auto &&...iters) {
            // No <auto> decay!
            // Example: zip(views::iota(1, 5), named_vector_of_int).
            // Return: std::tuple<int, int&>.
            return std::tuple<decltype(*iters)...>((*iters)...);
        }, _currents);
    }

    constexpr auto operator[](difference_type n) const {
        auto tmp = *this;
        tmp.operator+=(n);
        return tmp;
    }

    constexpr iterator& operator++() {
        return this->operator+=(1);
    }

    constexpr iterator operator++(int) {
        auto tmp = *this;
        this->operator+=(1);
        return tmp;
    }
    constexpr iterator& operator+=(difference_type n) {
        std::apply([&](auto &...iters) { ((iters += n),...); }, _currents);
        return *this;
    }

    friend constexpr auto operator<=>(const iterator &x, const iterator &y) = default;

private:
    std::tuple<std::ranges::iterator_t<Views>...> _currents;
};

template <std::ranges::input_range ...Views>
struct Zip<Views...>::sentinel {
    sentinel() = default;
    constexpr sentinel(Zip *this_zip) noexcept: _this_zip(this_zip) {}

    friend bool operator==(const iterator &x, const sentinel &y) {
        return [&]<auto ...Is>(std::index_sequence<Is...>) {
            return ((std::get<Is>(x._currents)
                        == std::ranges::end(std::get<Is>(y._this_zip->_views))) || ...);
        }(std::make_index_sequence<sizeof...(Views)>{});
    }

private:
    Zip *_this_zip;
};

inline constexpr struct Zip_fn {
    // template <std::ranges::input_range ...Rs>
    template <typename ...Rs>
    [[nodiscard]]
    constexpr auto operator()(Rs &&...rs) const {
        if constexpr (sizeof...(rs) == 0) {
            return std::views::empty<std::tuple<>>;
        } else {
            return Zip<std::views::all_t<Rs>...>(std::forward<Rs>(rs)...);
        }
    }
} zip;
}