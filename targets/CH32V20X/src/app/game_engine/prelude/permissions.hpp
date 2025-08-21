#pragma once

namespace ymd::fs{


struct Permissions{
    constexpr bool is_readonly() const{
        return is_readonly_;
    }

private:
    bool is_readonly_;
};


}