#pragma once

#include "../StringView.hpp"

#include <ranges>
#include <optional>

namespace ymd {

class StringView;

namespace views {

class SplitView : public std::ranges::view_interface<SplitView> {
private:
    StringView base_;
    char delimiter_;
    std::optional<size_t> max_pieces_;

public:
    SplitView(StringView base, char delimiter, std::optional<size_t> max_pieces = std::nullopt)
        : base_(base), delimiter_(delimiter), max_pieces_(max_pieces) {}

    class iterator {
    private:
        StringView current_;
        char delimiter_;
        size_t count_ = 0;
        std::optional<size_t> max_pieces_;

    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = StringView;

        iterator() = default;

        explicit iterator(StringView str, char delim, std::optional<size_t> max = std::nullopt)
            : current_(str), delimiter_(delim), max_pieces_(max) {}

        bool operator==(const iterator&) const = default;

        StringView operator*() const {
            auto pos = current_.find(delimiter_);
            if (pos == size_t(-1)) return current_;
            return current_.substr(0, pos);
        }

        iterator& operator++() {
            auto pos = current_.find(delimiter_);
            if (pos == size_t(-1)) {
                current_ = StringView{std::nullopt};
            } else {
                current_ = current_.substr(pos + 1);
            }
            if (max_pieces_) ++count_;
            return *this;
        }

        void operator++(int) { ++*this; }

        bool at_end() const { return current_.empty() || (max_pieces_ && count_ >= *max_pieces_); }
    };

    iterator begin() const { return iterator(base_, delimiter_, max_pieces_); }
    std::default_sentinel_t end() const { return {}; }
};

// Range adaptor object for `views::split`
struct SplitAdaptor {
    char delimiter_;
    std::optional<size_t> max_pieces_;

    template<std::ranges::viewable_range R>
    friend auto operator|(R&& r, const SplitAdaptor& adaptor) {
        return SplitView(StringView(r.data(), r.size()), adaptor.delimiter_, adaptor.max_pieces_);
    }
};

inline constexpr struct {
    template<typename... Args>
    constexpr auto operator()(Args&&... args) const {
        return SplitAdaptor{static_cast<char>(args)...};
    }
} split;

}  // namespace views


class SplitIterator {
public:
    using value_type = StringView;

    constexpr SplitIterator(const StringView& str, char delimiter, size_t max_pieces = 0)
        : data_{str.data()}, size_{str.size()}, delimiter_{delimiter}, max_pieces_{max_pieces}, remaining_{str.size_} {}

    constexpr std::optional<StringView> next() {
        if (remaining_ == 0 || (max_pieces_ != 0 && count_ >= max_pieces_)) {
            return std::nullopt;
        }

        const char* start = data_;
        const char* end = data_ + remaining_;
        const char* found = static_cast<const char*>(memchr(start, delimiter_, remaining_));

        if (found != nullptr) {
            StringView result(start, found - start);
            data_ = found + 1;
            remaining_ = end - data_;
            ++count_;
            return result;
        } else {
            // Last piece
            if (count_ < max_pieces_ || max_pieces_ == 0) {
                ++count_;
                remaining_ = 0;
                return StringView(start, end - start);
            } else {
                return std::nullopt;
            }
        }
    }

private:
    const char* data_;
    size_t size_;
    char delimiter_;
    size_t max_pieces_;
    size_t remaining_;
    size_t count_ = 0;
};

// Rust-style iterator factory method
static constexpr SplitIterator split(
    const StringView str, 
    char delimiter, 
    size_t max_pieces = 0
){
    return SplitIterator(str, delimiter, max_pieces);
}

}  // namespace ymd