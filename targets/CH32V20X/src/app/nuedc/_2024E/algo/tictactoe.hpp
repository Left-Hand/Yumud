#pragma once

#include "prelude.hpp"

using namespace ymd;

namespace nuedc::_2024E{


// 迭代下一步棋
// @param role 玩家角色
// @ChessBoard board 棋盘
// @return 最优的下一步棋的位置
[[nodiscard]] static constexpr Vector2u 
chess_forward_ai(const Role role, const ChessBoard & board){
    // 判断指定位置是否能让指定角色获胜
    auto is_winning_move = [&board](const Vector2u& pos, Role check_role) -> bool {
        {
            // 检查行
            size_t cnt = 0;
            for (size_t x = 0; x < ChessBoard::WIDTH; ++x) {
                if (board.at({x, pos.y}) == check_role) ++cnt;
            }
            if(cnt >= 2) return true;
        }

        {
            // 检查列
            size_t cnt = 0;
            for (size_t y = 0; y < ChessBoard::WIDTH; ++y) {
                if (board.at({pos.x, y}) == check_role) ++cnt;
            }
            if(cnt >= 2) return true;
        }

        {
            // 检查主对角线
            size_t cnt = 0;
            if (pos.x == pos.y) {
                for (size_t i = 0; i < ChessBoard::WIDTH; ++i) {
                    if (board.at({i, i}) == check_role) ++cnt;
                }
            }
            if(cnt >= 2) return true;
        }

        {
            // 检查副对角线
            size_t cnt = 0;
            if (pos.x + pos.y == 2) {
                for (size_t i = 0; i < ChessBoard::WIDTH; ++i) {
                    if (board.at({i, 2 - i}) == check_role) ++cnt;
                }
            }
            if(cnt >= 2) return true;
        }

        return false;
    };

    // 寻找可立即获胜的位置
    auto find_winning_move = [is_winning_move, &board](const Role check_role) -> std::optional<Vector2u> {
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


    // 1. 优先抢占必胜点
    if (const auto pos = find_winning_move(role)) return *pos;

    // 2. 阻止对手必胜
    if (const auto pos = find_winning_move(role.get_opponent())) return *pos;

    // 3. 占领中心
    if (board.at({1,1}) == None) return {1,1};

    // 4. 占领四个角落
    constexpr std::array<Vector2u, 4> corners = {{{0,0}, {0,2}, {2,0}, {2,2}}};
    for (const auto& pos : corners) {
        if (board.at(pos) == None) return pos;
    }

    // 5. 占领边缘（非中心非角落）
    constexpr std::array<Vector2u, 4> edges = {{{0,1}, {1,0}, {1,2}, {2,1}}};
    for (const auto& pos : edges) {
        if (board.at(pos) == None) return pos;
    }

    sys::abort();

    // 6. 最后兜底（理论上不会执行到此处）
    for (size_t y = 0; y < ChessBoard::WIDTH; ++y) {
        for (size_t x = 0; x < ChessBoard::WIDTH; ++x) {
            const Vector2u pos{x, y};
            if (board.at(pos) == None) return pos;
        }
    }

    return {0, 0}; // 安全返回（虽然理论上不会到达）
}

}