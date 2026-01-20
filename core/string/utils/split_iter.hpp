#pragma once

#include "core/string/view/string_view.hpp"
#include "core/utils/scope_guard.hpp"
#include <ranges>


namespace ymd{

struct StringSplitIter {
    constexpr explicit StringSplitIter(
        const StringView str, 
        const char delimiter
    ): 
        str_(str), 
        delimiter_(delimiter)
    {
        seek_next();
    }

    [[nodiscard]] constexpr bool has_next() const {
        return pos_ < str_.size();
    }

    constexpr Option<StringView> next() {
        auto guard = make_scope_guard([&]{
            pos_ = end_ + 1; // Skip the newline character
            seek_next();
        });
        return str_.substr_by_len(pos_, end_ - pos_);
    }

private:
    constexpr void seek_next() {
        end_ = pos_;
        while (end_ < str_.size() && str_[end_] != delimiter_) {
            ++end_;
        }
    }

    const StringView str_;
    const char delimiter_;
    size_t pos_   = 0;  // Start of current line
    size_t end_     = 0;    // End of current line (points to '\n' or str_.size())
};


class SplitIterator {
public:
    using value_type = StringView;

    constexpr SplitIterator(const StringView str, char delimiter, size_t max_pieces = 0)
        : data_{str.data()}, 
        size_{str.size()}, 
        delimiter_{delimiter}, 
        max_pieces_{max_pieces}, 
        remaining_{str.size()} {}

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

static constexpr SplitIterator split_str(
    const StringView str, 
    char delimiter, 
    size_t max_pieces = 0
){
    return SplitIterator(str, delimiter, max_pieces);
}


#if 0
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
#endif

}