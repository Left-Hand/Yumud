#include "tictactoe.hpp"
#include "minimax_tmp.hpp"

using namespace nuedc::_2024E;

#if 1

namespace unittest{


// template<typename Fn>
// static consteval void test_chess_forward(Fn && fn){
//     static_assert(std::forward<Fn>(fn)(Role::O, ChessBoard{{
//         std::to_array({Chess::N(), Chess::N(), Chess::N()}),
//         std::to_array({Chess::N(), Chess::N(), Chess::N()}),
//         std::to_array({Chess::N(), Chess::N(), Chess::N()}),
//     }}) == Vector2u{1,1});
// }

// cause gcc internal bug
// struct TestHelper{
//     template<typename Fn>
//     static constexpr void test(Fn && fn){

//     }
// };

static constexpr void test(){
    constexpr auto fn = chess_forward_ai;

    static_assert(fn(Role::O, ChessBoard{{
        std::to_array({Chess::N(), Chess::N(), Chess::N()}),
        std::to_array({Chess::N(), Chess::N(), Chess::N()}),
        std::to_array({Chess::N(), Chess::N(), Chess::N()}),
    }}) == Vector2u{1,1});
    
    static_assert(fn(Role::O, ChessBoard::from_str("NNNNNNNNN"
    ).unwrap()) == Vector2u{1,1});

    static_assert(fn(Role::O, ChessBoard{{
        std::to_array({Chess::N(), Chess::N(), Chess::N()}),
        std::to_array({Chess::N(), Chess::N(), Chess::N()}),
        std::to_array({Chess::O(), Chess::N(), Chess::O()}),
    }}) == Vector2u{1,2});

    // Test 1: Center capture priority
    static_assert(fn(Role::O, ChessBoard{{
        std::to_array({Chess::N(), Chess::N(), Chess::N()}),
        std::to_array({Chess::N(), Chess::N(), Chess::N()}),
        std::to_array({Chess::N(), Chess::N(), Chess::N()}),
    }}) == Vector2u{1,1}, "Center should be first choice");

    // Test 2: Winning move detection (Row)
    static_assert(fn(Role::X, ChessBoard{{
        std::to_array({Chess::X(), Chess::X(), Chess::N()}),
        std::to_array({Chess::N(), Chess::O(), Chess::N()}),
        std::to_array({Chess::N(), Chess::N(), Chess::O()}),
    }}) == Vector2u{2,0}, "Should complete winning row");

    // Test 3: Block opponent win (Column)
    static_assert(fn(Role::O, ChessBoard{{
        std::to_array({Chess::X(), Chess::N(), Chess::N()}),
        std::to_array({Chess::X(), Chess::O(), Chess::N()}),
        std::to_array({Chess::N(), Chess::N(), Chess::N()}),
    }}) == Vector2u{0,2}, "Should block opponent's column win");

    // Test 4: Corner preference when center taken
    static_assert(fn(Role::X, ChessBoard{{
        std::to_array({Chess::N(), Chess::N(), Chess::N()}),
        std::to_array({Chess::N(), Chess::O(), Chess::N()}),
        std::to_array({Chess::N(), Chess::N(), Chess::N()}),
    }}) == Vector2u{0,0}, "Corner should be second choice");

    // Test 5: Edge selection as last resort
    static_assert(fn(Role::O, ChessBoard{{
        std::to_array({Chess::X(), Chess::N(), Chess::X()}),
        std::to_array({Chess::N(), Chess::O(), Chess::N()}),
        std::to_array({Chess::X(), Chess::N(), Chess::O()}),
    }}) == Vector2u{1,0}, "Edge should be final fallback");

    // Test 6: Full board defense priority
    static_assert(fn(Role::X, ChessBoard{{
        std::to_array({Chess::X(), Chess::O(), Chess::X()}),
        std::to_array({Chess::O(), Chess::X(), Chess::N()}),
        std::to_array({Chess::N(), Chess::X(), Chess::O()}),
    }}) == Vector2u{0,2}, "Should prioritize blocking over offense");
}

}
#endif