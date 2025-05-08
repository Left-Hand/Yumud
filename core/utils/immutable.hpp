#pragma once

namespace ymd{

// template<typename T>
// class immutable_t{
// public:
//     constexpr immutable_t(const auto & value):
//     value_(static_cast<T>(value)){;}
    
//     constexpr immutable_t(auto && value):
//     value_(static_cast<T>(value)){;}
    
//     constexpr immutable_t & operator = (auto && value) = delete;

//     constexpr const T & get() const {
//         return value_;
//     }

//     constexpr T & borrow_mut(){
//         return value_;
//     }
// private:
//     T value_;
// };

}