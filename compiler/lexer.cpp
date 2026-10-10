#include "lexer.h"
#include <cctype>
#include <iostream>

namespace lore {

Lexer::Lexer(const std::string& filename, const std::string& source)
    : filename_(filename), source_(source) {}

char Lexer::peek(size_t offset) const {
    if (cursor_ + offset >= source_.size()) return '\0';
    return source_[cursor_ + offset];
}

char Lexer::advance() {
    if (is_at_end()) return '\0';
    char c = source_[cursor_++];
    if (c == '\n') {
        line_++;
        col_ = 1;
    } else {
        col_++;
    }
    return c;
}

bool Lexer::is_at_end() const {
    return cursor_ >= source_.size();
}

bool Lexer::match(const std::string& expected) {
    if (cursor_ + expected.size() > source_.size()) return false;
    for (size_t i = 0; i < expected.size(); ++i) {
        if (source_[cursor_ + i] != expected[i]) return false;
    }
    for (size_t i = 0; i < expected.size(); ++i) {
        advance();
    }
    return true;
}

void Lexer::skip_whitespace_and_comments() {
    while (!is_at_end()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
        } else if (c == '/' && peek(1) == '/' && peek(2) == '/') {
            // Comment /// until newline
            while (!is_at_end() && peek() != '\n') {
                advance();
            }
        } else {
            break;
        }
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (!is_at_end()) {
        skip_whitespace_and_comments();
        if (is_at_end()) break;

        Token tok = next_token();
        // Skip optional filler words "a" and "the"
        if (tok.type == TokenType::IDENT && (tok.text == "a" || tok.text == "the")) {
            continue;
        }
        tokens.push_back(tok);
    }

    tokens.push_back(Token{TokenType::END_OF_FILE, "", current_pos()});
    return tokens;
}

Token Lexer::next_token() {
    SourcePos pos = current_pos();
    char c = peek();

    // Check for "package:"
    if (source_.compare(cursor_, 8, "package:") == 0) {
        for (int i = 0; i < 8; ++i) advance();
        return Token{TokenType::KW_PACKAGE_COLON, "package:", pos};
    }

    // Check for "CS:" colour reference
    if (source_.compare(cursor_, 3, "CS:") == 0) {
        for (int i = 0; i < 3; ++i) advance();
        std::string ref = "CS:";
        while (!is_at_end() && (std::isalnum(peek()) || peek() == '_' || peek() == '-')) {
            ref += advance();
        }
        return Token{TokenType::COLOR_REF, ref, pos};
    }

    // Check for .end<handle>att. or DOT
    if (c == '.') {
        return lex_dot_or_attr_end();
    }

    if (c == ':') {
        advance();
        return Token{TokenType::COLON, ":", pos};
    }

    if (c == '[') {
        advance();
        return Token{TokenType::LBRACKET, "[", pos};
    }

    if (c == ']') {
        advance();
        return Token{TokenType::RBRACKET, "]", pos};
    }

    if (c == '(') {
        advance();
        return Token{TokenType::LPAREN, "(", pos};
    }

    if (c == ')') {
        advance();
        return Token{TokenType::RPAREN, ")", pos};
    }

    if (c == '#') {
        return lex_hex_color();
    }

    if (c == '"') {
        return lex_string();
    }

    if (std::isdigit(c)) {
        return lex_number();
    }

    if (std::isalpha(c) || c == '_') {
        return lex_identifier_or_keyword();
    }

    advance();
    throw CompileError(pos, std::string("Unexpected character '") + c + "'");
}

Token Lexer::lex_dot_or_attr_end() {
    SourcePos pos = current_pos();
    // Check if starts with .end
    if (source_.compare(cursor_, 4, ".end") == 0) {
        for (int i = 0; i < 4; ++i) advance();
        std::string handle;
        while (!is_at_end() && (std::isalnum(peek()) || peek() == '_' || peek() == '-')) {
            // Check if we hit "att."
            if (source_.compare(cursor_, 4, "att.") == 0) {
                break;
            }
            handle += advance();
        }
        if (source_.compare(cursor_, 4, "att.") == 0) {
            for (int i = 0; i < 4; ++i) advance();
            Token t{TokenType::ATTR_END, ".end" + handle + "att.", pos};
            t.handle = handle;
            return t;
        } else {
            throw CompileError(pos, "Malformed attribute end terminator: expected '.end<handle>att.'");
        }
    }

    advance();
    return Token{TokenType::DOT, ".", pos};
}

Token Lexer::lex_hex_color() {
    SourcePos pos = current_pos();
    std::string text;
    text += advance(); // '#'
    for (int i = 0; i < 6; ++i) {
        if (is_at_end() || !std::isxdigit(peek())) {
            throw CompileError(pos, "Invalid hex colour: expected 6 hex digits after '#'");
        }
        text += advance();
    }
    return Token{TokenType::HEX_COLOR, text, pos};
}

Token Lexer::lex_string() {
    SourcePos pos = current_pos();
    advance(); // opening '"'
    std::string content;
    while (!is_at_end() && peek() != '"') {
        if (peek() == '\\') {
            advance();
            if (is_at_end()) break;
            char esc = advance();
            if (esc == 'n') content += '\n';
            else if (esc == 't') content += '\t';
            else if (esc == '"') content += '"';
            else if (esc == '\\') content += '\\';
            else content += esc;
        } else {
            content += advance();
        }
    }

    if (is_at_end() || peek() != '"') {
        throw CompileError(pos, "Unterminated string literal");
    }
    advance(); // closing '"'
    return Token{TokenType::STRING, content, pos};
}

Token Lexer::lex_number() {
    SourcePos pos = current_pos();
    std::string num_str;
    while (!is_at_end() && std::isdigit(peek())) {
        num_str += advance();
    }

    // Check if decimal point followed by digit
    if (peek() == '.' && std::isdigit(peek(1))) {
        num_str += advance(); // '.'
        while (!is_at_end() && std::isdigit(peek())) {
            num_str += advance();
        }
    }

    double val = std::stod(num_str);

    // Check unit: "dp", "sp", "px", "%"
    if (source_.compare(cursor_, 2, "dp") == 0) {
        advance(); advance();
        Token t{TokenType::DIMENSION, num_str + "dp", pos};
        t.num_value = val;
        t.unit = "dp";
        return t;
    }
    if (source_.compare(cursor_, 2, "sp") == 0) {
        advance(); advance();
        Token t{TokenType::DIMENSION, num_str + "sp", pos};
        t.num_value = val;
        t.unit = "sp";
        return t;
    }
    if (source_.compare(cursor_, 2, "px") == 0) {
        advance(); advance();
        Token t{TokenType::DIMENSION, num_str + "px", pos};
        t.num_value = val;
        t.unit = "px";
        return t;
    }
    if (peek() == '%') {
        advance();
        Token t{TokenType::DIMENSION, num_str + "%", pos};
        t.num_value = val;
        t.unit = "%";
        return t;
    }

    Token t{TokenType::NUMBER, num_str, pos};
    t.num_value = val;
    return t;
}

Token Lexer::lex_identifier_or_keyword() {
    SourcePos pos = current_pos();

    // Check single-token block terminators
    const std::pair<const char*, TokenType> block_terminators[] = {
        {"EndOfPermission.", TokenType::END_OF_PERMISSION},
        {"EndOfHCCS.", TokenType::END_OF_HCCS},
        {"EndOfWhen.", TokenType::END_OF_WHEN},
        {"EndOfIf.", TokenType::END_OF_IF},
        {"EndOfRepeat.", TokenType::END_OF_REPEAT}
    };
    for (const auto& term : block_terminators) {
        size_t len = std::string(term.first).length();
        if (source_.compare(cursor_, len, term.first) == 0) {
            for (size_t i = 0; i < len; ++i) advance();
            return Token{term.second, term.first, pos};
        }
    }

    std::string text;
    while (!is_at_end() && (std::isalnum(peek()) || peek() == '_' || peek() == '-')) {
        text += advance();
    }

    // Keyword matching
    if (text == "package") return Token{TokenType::KW_PACKAGE, text, pos};
    if (text == "from") return Token{TokenType::KW_FROM, text, pos};
    if (text == "import") return Token{TokenType::KW_IMPORT, text, pos};
    if (text == "permission") return Token{TokenType::KW_PERMISSION, text, pos};
    if (text == "and") return Token{TokenType::KW_AND, text, pos};
    if (text == "or") return Token{TokenType::KW_OR, text, pos};
    if (text == "HCCS") return Token{TokenType::KW_HCCS, text, pos};
    if (text == "root") return Token{TokenType::KW_ROOT, text, pos};
    if (text == "is") return Token{TokenType::KW_IS, text, pos};
    if (text == "put") return Token{TokenType::KW_PUT, text, pos};
    if (text == "inside") return Token{TokenType::KW_INSIDE, text, pos};
    if (text == "state") return Token{TokenType::KW_STATE, text, pos};
    if (text == "when") return Token{TokenType::KW_WHEN, text, pos};
    if (text == "if") return Token{TokenType::KW_IF, text, pos};
    if (text == "then") return Token{TokenType::KW_THEN, text, pos};
    if (text == "otherwise") return Token{TokenType::KW_OTHERWISE, text, pos};
    if (text == "repeat") return Token{TokenType::KW_REPEAT, text, pos};
    if (text == "times") return Token{TokenType::KW_TIMES, text, pos};
    if (text == "add") return Token{TokenType::KW_ADD, text, pos};
    if (text == "to") return Token{TokenType::KW_TO, text, pos};
    if (text == "subtract") return Token{TokenType::KW_SUBTRACT, text, pos};
    if (text == "show") return Token{TokenType::KW_SHOW, text, pos};
    if (text == "play") return Token{TokenType::KW_PLAY, text, pos};
    if (text == "hide") return Token{TokenType::KW_HIDE, text, pos};
    if (text == "display") return Token{TokenType::KW_DISPLAY, text, pos};
    if (text == "at") return Token{TokenType::KW_AT, text, pos};
    if (text == "equal") return Token{TokenType::KW_EQUAL, text, pos};
    if (text == "greater") return Token{TokenType::KW_GREATER, text, pos};
    if (text == "less") return Token{TokenType::KW_LESS, text, pos};
    if (text == "than") return Token{TokenType::KW_THAN, text, pos};
    if (text == "not") return Token{TokenType::KW_NOT, text, pos};
    if (text == "plus") return Token{TokenType::KW_PLUS, text, pos};
    if (text == "minus") return Token{TokenType::KW_MINUS, text, pos};
    if (text == "divided") return Token{TokenType::KW_DIVIDED, text, pos};
    if (text == "by") return Token{TokenType::KW_BY, text, pos};
    if (text == "true") return Token{TokenType::KW_TRUE, text, pos};
    if (text == "false") return Token{TokenType::KW_FALSE, text, pos};

    // Support 'lable' as alias of 'label'
    if (text == "lable") {
        text = "label";
    }

    return Token{TokenType::IDENT, text, pos};
}

} // namespace lore
