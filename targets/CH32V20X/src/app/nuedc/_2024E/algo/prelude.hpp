#pragma once

#include "core/utils/Option.hpp"
#include "core/string/view/string_view.hpp"
#include "algebra/vectors/vec2.hpp"

using namespace ymd;

namespace nuedc::_2024E{


struct Role{
public:
    enum struct Kind:uint8_t{
        X,
        O
    };

    using enum Kind;

    [[nodiscard]]
    constexpr Role() = delete;
    
    [[nodiscard]]
    constexpr Role(const Kind kind) : kind_(kind){;}

    [[nodiscard]]
    constexpr Role(const Role & other) = default;

    [[nodiscard]]
    static constexpr Option<Role> from_char(const char chr){
        switch(chr){
            case 'X': return Some<Role>(X);
            case 'O': return Some<Role>(O);
            default: return None;
        }
    }

    [[nodiscard]]
    constexpr char to_char() const { 
        switch(kind_){
            case X: return 'X';
            case O: return 'O';
            default: __builtin_unreachable();
        }
    }

    [[nodiscard]]
    constexpr bool operator ==(const Kind kind) const{return kind_ == kind;}

    [[nodiscard]]
    constexpr bool operator !=(const Kind kind) const{return kind_ != kind;}

    [[nodiscard]]
    constexpr bool operator ==(const Role other) const{return kind_ == other.kind_;}

    [[nodiscard]]
    constexpr bool operator !=(const Role other) const{return kind_ != other.kind_;}

    [[nodiscard]] constexpr Role opponent() const{return Role(kind_ == X ? O : X);}

    [[nodiscard]] constexpr Kind kind() const {return kind_;}
private:
    Kind kind_;
};

struct [[nodiscard]] ChessCell final{
public:
    ChessCell() = delete;

    [[nodiscard]]
    constexpr ChessCell(const Role kind) : data_(Some(kind)){;}

    [[nodiscard]]
    constexpr ChessCell(const _None_t) : data_(None){;}

    [[nodiscard]]
    static constexpr ChessCell O() {return ChessCell(Role::O);}

    [[nodiscard]]
    static constexpr ChessCell X() {return ChessCell(Role::X);}

    [[nodiscard]]
    static constexpr ChessCell N() {return ChessCell(None);}

    [[nodiscard]]
    static constexpr Option<ChessCell> from_char(const char chr){
        switch(chr){
            case 'X': return Some(X());
            case 'O': return Some(O());
            case 'N': return Some(N());
            default: return None;
        }
    }

    [[nodiscard]]
    constexpr char to_char() const {
        if(data_.is_none()) return 'N';
        switch(data_.unwrap().kind()){
            case Role::X: return 'X';
            case Role::O: return 'O';
            default: sys::abort();
        }
    }

    [[nodiscard]]
    constexpr bool operator == (const Role kind) const {
        return data_.is_some() && data_.unwrap() == kind;
    }

    [[nodiscard]]
    constexpr bool operator != (const Role & role) const{
        return data_.is_some() && data_.unwrap() != role;
    } 
    
    [[nodiscard]]
    constexpr bool operator == (const ymd::_None_t) const {
        return data_.is_none();
    }

    [[nodiscard]]
    constexpr bool operator != (const ymd::_None_t) const{
        return data_.is_some();
    }

    [[nodiscard]]
    constexpr bool is_some() const {
        return data_.is_some();
    }
private:
    using Data = ymd::Option<Role>;
    Data data_ = None;
};

struct [[nodiscard]] ChessBoard final{
public:
    static constexpr size_t WIDTH = 3;
    using Row = std::array<ChessCell, WIDTH>;
    using Data = std::array<Row, WIDTH>;

    [[nodiscard]]
    constexpr ChessBoard(const ChessBoard & other) = default;

    [[nodiscard]]
    constexpr ChessBoard(ChessBoard && other) = default;

    [[nodiscard]]
    constexpr ChessBoard(const Data & data):
        data_(data){;}

    [[nodiscard]]
    static constexpr Option<ChessBoard> from_str(const StringView str){
        if(str.length() != WIDTH * WIDTH) sys::abort();
        auto build_row = [str](size_t row_index) constexpr -> Option<Row> {
            const auto c0_opt = ChessCell::from_char(str[row_index * WIDTH + 0]);
            if(c0_opt.is_none()) return None;
            const auto c1_opt = ChessCell::from_char(str[row_index * WIDTH + 1]);
            if(c1_opt.is_none()) return None;
            const auto c2_opt = ChessCell::from_char(str[row_index * WIDTH + 2]);
            if(c2_opt.is_none()) return None;

            return Some(Row{
                c0_opt.unwrap(),
                c1_opt.unwrap(),
                c2_opt.unwrap()
            });
        };

        const auto row0_opt = build_row(0);
        if(row0_opt.is_none()) return None;
        const auto row1_opt = build_row(1);
        if(row1_opt.is_none()) return None;
        const auto row2_opt = build_row(2);
        if(row2_opt.is_none()) return None;

        return Some(ChessBoard{Data{
            row0_opt.unwrap(),
            row1_opt.unwrap(),
            row2_opt.unwrap()
        }});
    }
    
    [[nodiscard]]
    constexpr auto & at(const math::Vec2u pos){
        if(not (pos.x < WIDTH and pos.y < WIDTH)) sys::abort();
        return data_[pos.y][pos.x];
    }
    
    [[nodiscard]]
    constexpr const auto & at(const math::Vec2u pos) const{
        if(not (pos.x < WIDTH and pos.y < WIDTH)) sys::abort();
        return data_[pos.y][pos.x];
    }

    [[nodiscard]]
    constexpr bool is_full() const {
        auto & self = *this;
        for (size_t y = 0; y < ChessBoard::WIDTH; ++y)
            for (size_t x = 0; x < ChessBoard::WIDTH; ++x)
                if (self.at({x, y}) == None) return false;
        return true;
    }

    [[nodiscard]] 
    constexpr ChessBoard remove_chess(const math::Vec2u pos) const {
        if(this->at({pos.x, pos.y}) == None) sys::abort();
        return modify(pos, ChessCell::N());
    }

    [[nodiscard]] 
    constexpr ChessBoard add_chess(const math::Vec2u pos, const Role role) const {
        if(this->at({pos.x, pos.y}) != None) sys::abort();
        return modify(pos, role == Role::X ? ChessCell::X() : ChessCell::O());
    }

    [[nodiscard]]
    constexpr ChessBoard modify(const math::Vec2u pos, const ChessCell chess) const {

        if(not (pos.x < WIDTH and pos.y < WIDTH)) sys::abort();
        auto data = data_;
        data[pos.y][pos.x] = chess;
        return {data};
    }

private:
    Data data_ = {
        std::array<ChessCell, WIDTH>{ChessCell{None}, ChessCell{None}, ChessCell{None}},
        std::array<ChessCell, WIDTH>{ChessCell{None}, ChessCell{None}, ChessCell{None}},
        std::array<ChessCell, WIDTH>{ChessCell{None}, ChessCell{None}, ChessCell{None}}
    };

    static constexpr auto DATA_SIZE = sizeof(Data);
};

}