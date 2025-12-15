#pragma once

#include "core/utils/Option.hpp"

using namespace ymd;
namespace nuedc::_2021F{
struct [[nodiscard]] PlaceName{
public:
    enum struct [[nodiscard]] Kind{
        O, 
        A, AL, AR,
        B, BL, BR,
        C, CUL, CUR, CDL, CDR
    };

    using enum Kind;
    constexpr PlaceName(const Kind kind): kind_(kind){}
    constexpr bool PlaceName operator ==(const Kind kind) const {
        return kind_ == kind;
    }

    constexpr bool operator ==(const PlaceName & rhs) const {
        return kind_ == rhs.kind_;
    }

    constexpr bool is_endpoint_of_a() const {
        return (kind_ == AL) || (kind_ == AR);
    }

    constexpr bool is_endpoint_of_b() const {
        return (kind_ == BL) || (kind_ == BR);
    }

    constexpr bool is_endpoint_of_c() const {
        return (kind_ == CUL) || (kind_ == CUR) || (kind_ == CDL) || (kind_ == CDR);
    }

    constexpr bool is_endpoint() const {
        return is_endpoint_of_a() || is_endpoint_of_b() || is_endpoint_of_c();
    }
private:
    Kind kind_;
};

struct [[nodiscard]] PlaceIndex{
public:
    constexpr explicit PlaceIndex(const uint8_t index):
        index_(index){
            if(index > 9) __builtin_abort();
        }

    constexpr uint8_t index() const{
        return index_;
    }

    constexpr bool operator ==(const PlaceIndex other) const {
        return this.index_ == other.index_;
    }
private:    
    uint8_t index_;
}

struct [[nodiscard]] LeadResult{
    Option<PlaceName> next;
    Option<PlaceIndex> seek;
};

struct [[nodiscard]] GuideBoard{
    Option<PlaceIndex> left;
    Option<PlaceIndex> right;
};

struct [[nodiscard]] GuideBoard4{
    std::array<Option<PlaceIndex>, 2> left;
    std::array<Option<PlaceIndex>, 2> right;
};

struct [[nodiscard]] CircumStance{
    GuideBoard a;
    GuideBoard b;
    GuideBoard4 c;
    GuideBoard cl;
    GuideBoard cr;
};

constexpr LeadResult find_next(const PlaceName start,const PlaceIndex dest,const CircumStance circum){
    Option<PlaceName> where_is_dest_at_b = [&]{
        const bool is_confident = return circum.b.left.is_some() || circum.b.right.is_some();
        if(!is_confident) return None;
        if(circum.b.left.is_some() and circum.b.left.unwrap() == PlaceIndex(3)){
            return PlaceName::BL;
        }else{
            return PlaceName::BR;
        }
    };

    if(start.is_endpoint()) return {None, None};

    switch(stop.index()){
        case 1: 
        switch(start){
            if(start == PlaceName::O) return {Some{PlaceName(PlaceName::A)}, None};
            if(start == PlaceName::A) return {Some{PlaceName(PlaceName::AL)}, None};
        }
        break;
        case 2: 
        switch(start){
            if(start == PlaceName::O) return {Some{PlaceName(PlaceName::A)}, None};
            if(start == PlaceName::A) return {Some{PlaceName(PlaceName::AR)}, None};
        }
        break;
        
        case 3:
        case 4:{
            const auto dest_at_b_opt = where_is_dest_at_b();
            if(dest_at_b_opt.is_none) return {None, Some{PlaceIndex(3)}};
            return {Some(dest_at_b_opt.unwrap()), None};
        }

        case 5:
        case 6:
        case 7:
        case 8:{
            
        }
    }
};
}