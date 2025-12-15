#pragma once

#include "core/stream/ostream.hpp"
#include "core/string/string_view.hpp"


namespace ymd{


template<size_t N>
class FixedStringStream final:public OutputStream{
public:
    FixedStringStream() {
        if (N > 0) {
            buf_[0] = '\0';  // Ensure null termination
        }
        this->force_sync(EN);
    }

    size_t free_capacity() const {
        return N - length_;
    }

    void sendout(const std::span<const char> pbuf) {
        // Check bounds to prevent overflow
        if (length_ + pbuf.size() >= N) {
            // Handle overflow - could throw exception or truncate
            // For now, we'll truncate to fit available space
            size_t available = N - 1 - length_;  // -1 for null terminator
            if (available > 0) {
                std::copy(pbuf.begin(), pbuf.begin() + available, buf_.data() + length_);
                length_ += available;
            }
        } else {
            std::copy(pbuf.begin(), pbuf.end(), buf_.data() + length_);
            length_ += pbuf.size();
        }
        
        // Ensure null termination
        if (length_ < N) {
            buf_[length_] = '\0';
        }
    }


    [[nodiscard]] constexpr size_t length() const {
        return length_;
    }

    [[nodiscard]] constexpr StringView str() const {
        return StringView(buf_.data(), length_);
    }

private:
    std::array<char, N> buf_;
    size_t length_ = 0;
};

};
