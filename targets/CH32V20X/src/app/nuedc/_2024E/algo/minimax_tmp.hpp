#pragma once

#include "tictactoe.hpp"

#if 0
namespace nuedc::_2024E{

// 判断棋盘是否已满（constexpr Lambda）


// 棋盘扩展操作（constexpr 方法）
struct BoardHelper {
    static constexpr ChessBoard with_move(const ChessBoard& b, Vec2u pos, Role role) {
        ChessBoard::Data new_data{};
        for (size_t y = 0; y < ChessBoard::WIDTH; ++y) {
            for (size_t x = 0; x < ChessBoard::WIDTH; ++x) {
                new_data[y][x] = (x == pos.x && y == pos.y) 
                    ? Chess(role) 
                    : b.at({x, y});
            }
        }
        return ChessBoard(new_data);
    }
};

// Minimax 核心模板（编译时展开）
template<bool IsMaximizing>
struct Minimax {
    static constexpr int run(const ChessBoard& b, Role current_role) {
        int score = evaluate(b);
        if (score != 0) return score;
        if (b.is_full()) return 0;

        int best_value = IsMaximizing ? -1000 : 1000;
        for (size_t y = 0; y < ChessBoard::WIDTH; ++y) {
            for (size_t x = 0; x < ChessBoard::WIDTH; ++x) {
                Vec2u pos{x, y};
                if (b.at(pos) == None) {
                    ChessBoard next_board = BoardHelper::with_move(b, pos, 
                        IsMaximizing ? current_role : current_role.get_opponent());
                    
                    int current_score = Minimax<!IsMaximizing>::run(
                        next_board, current_role.get_opponent());
                    
                    best_value = IsMaximizing 
                        ? std::max(best_value, current_score) 
                        : std::min(best_value, current_score);
                }
            }
        }
        return best_value;
    }
};

// 显式特化终止递归
template<>
struct Minimax<true> {
    static constexpr int run(const ChessBoard& b, Role current_role) {
        int score = evaluate(b);
        if (score != 0) return score;
        if (b.is_full()) return 0;

        int best = -1000;
        for (size_t y = 0; y < ChessBoard::WIDTH; ++y) {
            for (size_t x = 0; x < ChessBoard::WIDTH; ++x) {
                Vec2u pos{x, y};
                if (b.at(pos) == None) {
                    ChessBoard next_board = BoardHelper::with_move(b, pos, current_role);
                    best = std::max(best, Minimax<false>::run(next_board, current_role.get_opponent()));
                }
            }
        }
        return best;
    }
};

template<>
struct Minimax<false> {
    static constexpr int run(const ChessBoard& b, Role current_role) {
        int score = evaluate(b);
        if (score != 0) return score;
        if (b.is_full()) return 0;

        int best = 1000;
        for (size_t y = 0; y < ChessBoard::WIDTH; ++y) {
            for (size_t x = 0; x < ChessBoard::WIDTH; ++x) {
                Vec2u pos{x, y};
                if (b.at(pos) == None) {
                    ChessBoard next_board = BoardHelper::with_move(b, pos, current_role.get_opponent());
                    best = std::min(best, Minimax<true>::run(next_board, current_role.get_opponent()));
                }
            }
        }
        return best;
    }
};

// 迭代下一步棋
// @param role 玩家角色
// @ChessBoard board 棋盘
// @return 最优的下一步棋的位置
static constexpr Vec2u chess_forward_minimax(const Role role, const ChessBoard& board) {
    // 获取对手角色（constexpr Lambda）
    constexpr auto get_opponent = [](Role r) {
        return r == Role::X ? Role::O : Role::X;
    };

    // 判断游戏是否结束并返回分数（constexpr Lambda）
    constexpr auto evaluate = [](const ChessBoard& b) -> int {
        // 检查行/列/对角线是否有三连
        for (size_t i = 0; i < ChessBoard::WIDTH; ++i) {
            // 检查列
            if ((b.at({i, 0}) == Role::X && b.at({i, 1}) == Role::X && b.at({i, 2}) == Role::X)) return 1;
            // 检查行
            if ((b.at({0, i}) == Role::X && b.at({1, i}) == Role::X && b.at({2, i}) == Role::X)) return 1;
            // 检查列(O)
            if ((b.at({i, 0}) == Role::O && b.at({i, 1}) == Role::O && b.at({i, 2}) == Role::O)) return -1;
            // 检查行(O)
            if ((b.at({0, i}) == Role::O && b.at({1, i}) == Role::O && b.at({2, i}) == Role::O)) return -1;
        }
        // 主对角线(X)
        if (b.at({0,0}) == Role::X && b.at({1,1}) == Role::X && b.at({2,2}) == Role::X) return 1;
        // 副对角线(X)
        if (b.at({0,2}) == Role::X && b.at({1,1}) == Role::X && b.at({2,0}) == Role::X) return 1;
        // 主对角线(O)
        if (b.at({0,0}) == Role::O && b.at({1,1}) == Role::O && b.at({2,2}) == Role::O) return -1;
        // 副对角线(O)
        if (b.at({0,2}) == Role::O && b.at({1,1}) == Role::O && b.at({2,0}) == Role::O) return -1;
        return 0;
    };



    // 寻找最佳移动位置
    Vec2u best_pos{0, 0};
    int best_score = -1000;

    for (size_t y = 0; y < ChessBoard::WIDTH; ++y) {
        for (size_t x = 0; x < ChessBoard::WIDTH; ++x) {
            Vec2u pos{x, y};
            if (board.at(pos) == None) {
                ChessBoard next_board = BoardHelper::with_move(board, pos, role);
                int score = Minimax<false>::run(next_board, role.get_opponent());
                
                if (score > best_score) {
                    best_score = score;
                    best_pos = pos;
                }
            }
        }
    }

    return best_pos;
}

}

#endif