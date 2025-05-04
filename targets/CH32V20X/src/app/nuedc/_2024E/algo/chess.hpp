#pragma once

#include "core/utils/Option.hpp"
#include "types/vector2/Vector2.hpp"

using namespace ymd;

namespace nuedc::_2024E{


class Role{
public:
    enum class Kind:uint8_t{
        X,
        O
    };

    using enum Kind;

    //FIXME remove default constructor
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
            default: sys::abort();
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

    [[nodiscard]]
    constexpr Role operator !() const{return Role(kind_ == X ? O : X);}

    [[nodiscard]]
    constexpr Role get_opponent() const{return !(*this);}

    [[nodiscard]]
    constexpr Kind kind() const {return kind_;}
private:
    Kind kind_;
};

class Chess{
public:
    Chess() = delete;

    [[nodiscard]]
    constexpr Chess(const Role kind) : data_(Some(kind)){;}

    [[nodiscard]]
    constexpr Chess(const _None_t) : data_(None){;}

    [[nodiscard]]
    static constexpr Chess O() {return Chess(Role::O);}

    [[nodiscard]]
    static constexpr Chess X() {return Chess(Role::X);}

    [[nodiscard]]
    static constexpr Chess N() {return Chess(None);}

    [[nodiscard]]
    static constexpr Option<Chess> from_char(const char chr){
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

class ChessBoard{
public:
    static constexpr size_t WIDTH = 3;
    using Row = std::array<Chess, WIDTH>;
    using Data = std::array<Row, WIDTH>;

    [[nodiscard]]
    constexpr ChessBoard(const ChessBoard & other) = default;

    [[nodiscard]]
    constexpr ChessBoard(ChessBoard && other) = default;

    [[nodiscard]]
    constexpr ChessBoard(const Data & data):
        data_(data){;}

    [[nodiscard]]
    static constexpr Option<ChessBoard> from_str(const char * str){
        if(strlen(str) != WIDTH * WIDTH) sys::abort();
        auto build_row = [str](size_t row_index) constexpr -> Option<Row> {
            const auto c0_opt = Chess::from_char(str[row_index * WIDTH + 0]);
            if(c0_opt.is_none()) return None;
            const auto c1_opt = Chess::from_char(str[row_index * WIDTH + 1]);
            if(c1_opt.is_none()) return None;
            const auto c2_opt = Chess::from_char(str[row_index * WIDTH + 2]);
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
    constexpr auto & at(const Vector2u pos){
        if(not (pos.x < WIDTH and pos.y < WIDTH)) sys::abort();
        return data_[pos.y][pos.x];
    }
    
    [[nodiscard]]
    constexpr const auto & at(const Vector2u pos) const{
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
    constexpr ChessBoard remove_chess(const Vector2u pos) const {
        if(this->at({pos.x, pos.y}) == None) sys::abort();
        return modify(pos, Chess::N());
    }

    [[nodiscard]] 
    constexpr ChessBoard add_chess(const Vector2u pos, const Role role) const {
        if(this->at({pos.x, pos.y}) != None) sys::abort();
        return modify(pos, role == Role::X ? Chess::X() : Chess::O());
    }

    [[nodiscard]]
    constexpr ChessBoard modify(const Vector2u pos, const Chess chess) const {

        if(not (pos.x < WIDTH and pos.y < WIDTH)) sys::abort();
        auto data = data_;
        data[pos.y][pos.x] = chess;
        return {data};
    }

private:
    Data data_ = {
        std::array<Chess, WIDTH>{Chess{None}, Chess{None}, Chess{None}},
        std::array<Chess, WIDTH>{Chess{None}, Chess{None}, Chess{None}},
        std::array<Chess, WIDTH>{Chess{None}, Chess{None}, Chess{None}}
    };

    static constexpr auto DATA_SIZE = sizeof(Data);
};

}