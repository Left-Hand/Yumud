#include <string_view>
#include <cstdint>

namespace ymd::str {


// Specific	-v, --help	"-v", "--help"	必须以 - 开头
// Equals	=	"="	单字符
// Colon	:	":"	单字符
// String	"a b c"	"a b c"	带引号，内容不含引号
// String	plain, key:value, 123	"plain", "key:value", "123"	无引号，遇空格/=/: 停止

enum class [[nodiscard]] TokenKind:uint8_t {
    Specific,        // 
    Equals,          // =
    Colon,           // :
    String,          // 普通 / 引号字符串（值）
    Unknown
};

struct [[nodiscard]] Token final{
    TokenKind kind;
    std::string_view lexeme;   // 指向输入中的原始片段
};

class [[nodiscard]] Lexer final{
public:
    constexpr explicit Lexer(std::string_view input) : 
        cursor_(input.data()), 
        end_(input.data() + input.size()) {}

    // 修复：has_next 应在 cursor_ < end_ 时返回 true
    constexpr bool has_next() const {
        return cursor_ < end_;
    }

    constexpr Token next() {
        skip_whitespace();

        if (!has_next()) {
            __builtin_trap();
        }

        // 选项：以 '-' 开头
        if (*cursor_ == '-') {
            auto start = cursor_;
            while (cursor_ < end_ && !is_whitespace() && *cursor_ != '=' && *cursor_ != ':') {
                ++cursor_;
            }
            return Token{TokenKind::Specific, std::string_view(start, cursor_ - start)};
        }

        // 等号和冒号
        if (*cursor_ == '=') {
            ++cursor_;
            return Token{TokenKind::Equals, "="};
        }

        if (*cursor_ == ':') {
            ++cursor_;
            return Token{TokenKind::Colon, ":"};
        }

        // 引号字符串
        if (*cursor_ == '"') {
            ++cursor_;
            auto start = cursor_;
            while (cursor_ < end_ && *cursor_ != '"') {
                ++cursor_;
            }
            std::string_view lexeme(start, cursor_ - start);
            if (cursor_ < end_) {
                ++cursor_;
            }
            return Token{TokenKind::String, lexeme};
        }

        // 普通无引号字符串（直到空格、=、: 或结尾）
        auto start = cursor_;
        while (cursor_ < end_ && !is_whitespace() && *cursor_ != '=' && *cursor_ != ':') {
            ++cursor_;
        }
        return Token{TokenKind::String, std::string_view(start, cursor_ - start)};
    }

private:
    const char* cursor_;
    const char* end_;

    constexpr bool is_whitespace() const {
        return cursor_ < end_ && (*cursor_ == ' ' || *cursor_ == '\t');
    }

    constexpr void skip_whitespace() {
        while (cursor_ < end_ && is_whitespace()) {
            ++cursor_;
        }
    }
};




} // namespace ymd::str