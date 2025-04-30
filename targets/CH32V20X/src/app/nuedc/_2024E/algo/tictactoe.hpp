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
    constexpr Role() = default;
    
    [[nodiscard]]
    constexpr Role(const Kind kind) : kind_(kind){;}

    [[nodiscard]]
    constexpr Role(const Role & other) = default;

    [[nodiscard]]
    constexpr bool operator ==(const Kind kind) const{return kind_ == kind;}

    [[nodiscard]]
    constexpr bool operator !=(const Kind kind) const{return kind_ != kind;}

    [[nodiscard]]
    constexpr bool operator ==(const Role other) const{return kind_ == other.kind_;}

    [[nodiscard]]
    constexpr bool operator !=(const Role other) const{return kind_ == other.kind_;}

    [[nodiscard]]
    constexpr Role operator !() const{return Role(kind_ == X ? O : X);}

    [[nodiscard]]
    constexpr Role get_opponent() const{return !(*this);}
private:
    //FIXME remove default constructor
    Kind kind_ = X;
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
    constexpr bool operator == (const Role kind) const {
        return data_.is_some() && data_.unwrap() == kind;
    }

    [[nodiscard]]
    constexpr bool operator != (const Role & chess) const{
        return data_.is_some() && data_.unwrap() != chess;
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
    using Data = std::array<std::array<Chess, WIDTH>, WIDTH>;

    [[nodiscard]]
    constexpr ChessBoard() = default;

    [[nodiscard]]
    constexpr ChessBoard(const Data & data):
        data_(data){;}

    [[nodiscard]]
    constexpr auto & at(const Vector2u pos){
        if(pos.x >= WIDTH || pos.y >= WIDTH) __builtin_abort();
        return data_[pos.y][pos.x];
    }
    
    [[nodiscard]]
    constexpr const auto & at(const Vector2u pos) const{
        if(pos.x >= WIDTH || pos.y >= WIDTH) __builtin_abort();
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
private:


    Data data_ = {
        std::array<Chess, WIDTH>{Chess{None}, Chess{None}, Chess{None}},
        std::array<Chess, WIDTH>{Chess{None}, Chess{None}, Chess{None}},
        std::array<Chess, WIDTH>{Chess{None}, Chess{None}, Chess{None}}
    };

    static constexpr auto DATA_SIZE = sizeof(Data);
};


// 迭代下一步棋
// @param role 玩家角色
// @ChessBoard board 棋盘
// @return 最优的下一步棋的位置
[[nodiscard]] static constexpr Vector2u 
chess_forward_ai(const Role role, const ChessBoard & board){
    // 判断指定位置是否能让指定角色获胜
    auto is_winning_move = [&](const Vector2u& pos, Role check_role) -> bool {
        // 检查行
        int row_count = 0;
        for (size_t x = 0; x < ChessBoard::WIDTH; ++x) {
            if (board.at({x, pos.y}) == check_role) ++row_count;
        }
        
        // 检查列
        int col_count = 0;
        for (size_t y = 0; y < ChessBoard::WIDTH; ++y) {
            if (board.at({pos.x, y}) == check_role) ++col_count;
        }
        
        // 检查主对角线
        int diag_count = 0;
        if (pos.x == pos.y) {
            for (size_t i = 0; i < ChessBoard::WIDTH; ++i) {
                if (board.at({i, i}) == check_role) ++diag_count;
            }
        }
        
        // 检查副对角线
        int anti_diag_count = 0;
        if (pos.x + pos.y == 2) {
            for (size_t i = 0; i < ChessBoard::WIDTH; ++i) {
                if (board.at({i, 2 - i}) == check_role) ++anti_diag_count;
            }
        }
        
        return row_count >= 2 || col_count >= 2 || diag_count >= 2 || anti_diag_count >= 2;
    };

    // 寻找可立即获胜的位置
    auto find_winning_move = [&](Role check_role) -> std::optional<Vector2u> {
        for (size_t y = 0; y < ChessBoard::WIDTH; ++y) {
            for (size_t x = 0; x < ChessBoard::WIDTH; ++x) {
                Vector2u pos{x, y};
                if (board.at(pos) == None && is_winning_move(pos, check_role)) {
                    return pos;
                }
            }
        }
        return std::nullopt;
    };

    // 获取对手角色
    auto get_opponent = [&](Role r) { return r == Role::X ? Role::O : Role::X; };

    // 1. 优先抢占必胜点
    if (auto pos = find_winning_move(role)) return *pos;

    // 2. 阻止对手必胜
    if (auto pos = find_winning_move(get_opponent(role))) return *pos;

    // 3. 占领中心
    if (board.at({1,1}) == None) return {1,1};

    // 4. 占领四个角落
    const std::array<Vector2u, 4> corners = {{{0,0}, {0,2}, {2,0}, {2,2}}};
    for (const auto& pos : corners) {
        if (board.at(pos) == None) return pos;
    }

    // 5. 占领边缘（非中心非角落）
    const std::array<Vector2u, 4> edges = {{{0,1}, {1,0}, {1,2}, {2,1}}};
    for (const auto& pos : edges) {
        if (board.at(pos) == None) return pos;
    }

    // 6. 最后兜底（理论上不会执行到此处）
    for (size_t y = 0; y < ChessBoard::WIDTH; ++y) {
        for (size_t x = 0; x < ChessBoard::WIDTH; ++x) {
            Vector2u pos{x, y};
            if (board.at(pos) == None) return pos;
        }
    }

    return {0, 0}; // 安全返回（虽然理论上不会到达）
}

}