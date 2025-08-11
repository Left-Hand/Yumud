#pragma once

#include "core/stream/ostream.hpp"
#include "core/string/string_view.hpp"


namespace ymd{


template<size_t N>
class FixedStringStream:public OutputStream{
protected:
    char buf_[N];
    size_t length_ = 0;
    
public:
    FixedStringStream() {
        if (N > 0) {
            buf_[0] = '\0';  // Ensure null termination
        }
        this->force_sync();
    }

    size_t pending() const {
        return 0;
    }

    void sendout(const std::span<const char> pbuf) {
        // Check bounds to prevent overflow
        if (length_ + pbuf.size() >= N) {
            // Handle overflow - could throw exception or truncate
            // For now, we'll truncate to fit available space
            size_t available = N - 1 - length_;  // -1 for null terminator
            if (available > 0) {
                std::copy(pbuf.begin(), pbuf.begin() + available, buf_ + length_);
                length_ += available;
            }
        } else {
            std::copy(pbuf.begin(), pbuf.end(), buf_ + length_);
            length_ += pbuf.size();
        }
        
        // Ensure null termination
        if (length_ < N) {
            buf_[length_] = '\0';
        }
    }

    const char* c_str() const {
        return buf_;
    }

    constexpr size_t length() const {
        return length_;
    }

    explicit constexpr operator StringView() const {
        return StringView(c_str(), length());
    }
};

};
