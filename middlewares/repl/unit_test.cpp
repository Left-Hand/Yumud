#include "lexer.hpp"


using namespace ymd::str;

namespace{

static constexpr auto get_nth(Lexer && lexer, const size_t n) -> Token{
    for(size_t i = 0; i < n; i++){
        (void)lexer.next();
    }
    return lexer.next();
};


[[maybe_unused]] static void demo_lexer(){
    // demo: 简单的分词示例
    std::string_view input = "-v --filter key=value name:123 \"hello world\"";
    Lexer lexer(input);

    // 手动分词演示（constexpr版本）
    static constexpr std::string_view demo_input = "-v key=value";
    static constexpr Lexer demo_lexer(demo_input);
}

[[maybe_unused]] static void test_lexer(){


    // 测试1: 选项
    {
        static constexpr std::string_view s = "--filter";
        static constexpr auto tok = get_nth(Lexer(s), 0);
        static_assert(tok.kind == TokenKind::Specific);
        static_assert(tok.lexeme == "--filter");
    }

    // 测试2: 短选项
    {
        static constexpr std::string_view s = "-v";
        static constexpr auto tok = get_nth(Lexer(s), 0);
        static_assert(tok.kind == TokenKind::Specific);
        static_assert(tok.lexeme == "-v");
    }

    // 测试3: 等号
    {
        static constexpr std::string_view s = "=";
        static constexpr auto tok = get_nth(Lexer(s), 0);
        static_assert(tok.kind == TokenKind::Equals);
        static_assert(tok.lexeme == "=");
    }

    // 测试4: 冒号
    {
        static constexpr std::string_view s = ":";
        static constexpr auto tok = get_nth(Lexer(s), 0);
        static_assert(tok.kind == TokenKind::Colon);
        static_assert(tok.lexeme == ":");
    }

    // 测试5: 引号字符串
    {
        static constexpr std::string_view s = "\"hello world\"";
        static constexpr auto tok = get_nth(Lexer(s), 0);
        static_assert(tok.kind == TokenKind::String);
        static_assert(tok.lexeme == "hello world");
    }

    // 测试6: 普通无引号字符串
    {
        static constexpr std::string_view s = "plain";
        static constexpr auto tok = get_nth(Lexer(s), 0);
        static_assert(tok.kind == TokenKind::String);
        static_assert(tok.lexeme == "plain");
    }

    // 测试7: 多个token序列
    {
        static constexpr std::string_view s = "--filter key=value";

        {
            static constexpr auto tok = get_nth(Lexer(s), 0);
    
            static_assert(tok.kind == TokenKind::Specific);
            static_assert(tok.lexeme == "--filter");
        }

        {
            static constexpr auto tok = get_nth(Lexer(s), 1);
    
            static_assert(tok.kind == TokenKind::String);
            static_assert(tok.lexeme == "key");
        }
    }

    // 测试8: 带空格的多tokens
    {
        static constexpr std::string_view s = "key : value";
        static constexpr auto tok1 = get_nth(Lexer(s), 0);
        static_assert(tok1.kind == TokenKind::String);
        static_assert(tok1.lexeme == "key");
    }
}
}

