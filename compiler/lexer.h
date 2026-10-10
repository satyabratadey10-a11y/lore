#ifndef LORE_LEXER_H
#define LORE_LEXER_H

#include "token.h"
#include "source_pos.h"
#include <string>
#include <vector>

namespace lore {

class Lexer {
public:
    Lexer(const std::string& filename, const std::string& source);

    std::vector<Token> tokenize();

private:
    std::string filename_;
    std::string source_;
    size_t cursor_ = 0;
    int line_ = 1;
    int col_ = 1;

    char peek(size_t offset = 0) const;
    char advance();
    bool match(const std::string& expected);
    bool is_at_end() const;

    void skip_whitespace_and_comments();
    Token next_token();

    Token lex_number();
    Token lex_string();
    Token lex_identifier_or_keyword();
    Token lex_dot_or_attr_end();
    Token lex_hex_color();

    SourcePos current_pos() const {
        return SourcePos{filename_, line_, col_};
    }
};

} // namespace lore

#endif // LORE_LEXER_H
