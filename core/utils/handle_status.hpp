#pragma once

namespace ymd{

struct HandleStatus{
    static constexpr HandleStatus from_handled() { return HandleStatus{true}; }
    static constexpr HandleStatus from_unhandled() { return HandleStatus{false}; }

    bool is_handled() const { return is_handled_; }
private:
    constexpr HandleStatus(bool is_handled) : is_handled_(is_handled) {}

    bool is_handled_;
};
}