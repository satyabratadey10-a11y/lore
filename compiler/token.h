#ifndef LORE_TOKEN_H
#define LORE_TOKEN_H

#include "source_pos.h"
#include <string>

namespace lore {

enum class TokenType {
    END_OF_FILE,

    // Punctuation
    DOT,            // .
    COLON,          // :
    LBRACKET,       // [
    RBRACKET,       // ]
    LPAREN,         // (
    RPAREN,         // )

    // Literals
    NUMBER,         // 123, 3.14
    DIMENSION,      // 200dp, 64dp, 32sp, 100%
    STRING,         // "..."
    HEX_COLOR,      // #4287f5
    COLOR_REF,      // CS:blue

    // Identifier
    IDENT,          // names, handles

    // Block Terminators (Single tokens)
    END_OF_PERMISSION,  // EndOfPermission.
    END_OF_HCCS,        // EndOfHCCS.
    END_OF_WHEN,        // EndOfWhen.
    END_OF_IF,          // EndOfIf.
    END_OF_REPEAT,      // EndOfRepeat.
    ATTR_END,           // .end<handle>att.

    // Keywords
    KW_PACKAGE_COLON,   // package:
    KW_PACKAGE,         // package
    KW_FROM,            // from
    KW_IMPORT,          // import
    KW_PERMISSION,      // permission
    KW_AND,             // and
    KW_OR,              // or
    KW_HCCS,            // HCCS
    KW_ROOT,            // root
    KW_IS,              // is
    KW_PUT,             // put
    KW_INSIDE,          // inside
    KW_STATE,           // state
    KW_WHEN,            // when
    KW_IF,              // if
    KW_THEN,            // then
    KW_OTHERWISE,       // otherwise
    KW_REPEAT,          // repeat
    KW_TIMES,           // times
    KW_ADD,             // add
    KW_TO,              // to
    KW_SUBTRACT,        // subtract
    KW_SHOW,            // show
    KW_PLAY,            // play
    KW_HIDE,            // hide
    KW_DISPLAY,         // display
    KW_AT,              // at
    KW_EQUAL,           // equal
    KW_GREATER,         // greater
    KW_LESS,            // less
    KW_THAN,            // than
    KW_NOT,             // not
    KW_PLUS,            // plus
    KW_MINUS,           // minus
    KW_DIVIDED,         // divided
    KW_BY,              // by
    KW_TRUE,            // true
    KW_FALSE            // false
};

struct Token {
    TokenType type = TokenType::END_OF_FILE;
    std::string text;
    SourcePos pos;

    // Additional data
    double num_value = 0.0;
    std::string unit;     // for DIMENSION: "dp", "sp", "px", "%"
    std::string handle;   // for ATTR_END: extracted handle name

    std::string type_name() const {
        switch (type) {
            case TokenType::END_OF_FILE: return "end of file";
            case TokenType::DOT: return "'.'";
            case TokenType::COLON: return "':'";
            case TokenType::LBRACKET: return "'['";
            case TokenType::RBRACKET: return "']'";
            case TokenType::LPAREN: return "'('";
            case TokenType::RPAREN: return "')'";
            case TokenType::NUMBER: return "number";
            case TokenType::DIMENSION: return "dimension";
            case TokenType::STRING: return "string";
            case TokenType::HEX_COLOR: return "hex colour";
            case TokenType::COLOR_REF: return "colour reference (CS:...)";
            case TokenType::IDENT: return "identifier ('" + text + "')";
            case TokenType::END_OF_PERMISSION: return "'EndOfPermission.'";
            case TokenType::END_OF_HCCS: return "'EndOfHCCS.'";
            case TokenType::END_OF_WHEN: return "'EndOfWhen.'";
            case TokenType::END_OF_IF: return "'EndOfIf.'";
            case TokenType::END_OF_REPEAT: return "'EndOfRepeat.'";
            case TokenType::ATTR_END: return "attribute end ('.end" + handle + "att.')";
            case TokenType::KW_PACKAGE_COLON: return "'package:'";
            case TokenType::KW_PACKAGE: return "'package'";
            case TokenType::KW_FROM: return "'from'";
            case TokenType::KW_IMPORT: return "'import'";
            case TokenType::KW_PERMISSION: return "'permission'";
            case TokenType::KW_AND: return "'and'";
            case TokenType::KW_OR: return "'or'";
            case TokenType::KW_HCCS: return "'HCCS'";
            case TokenType::KW_ROOT: return "'root'";
            case TokenType::KW_IS: return "'is'";
            case TokenType::KW_PUT: return "'put'";
            case TokenType::KW_INSIDE: return "'inside'";
            case TokenType::KW_STATE: return "'state'";
            case TokenType::KW_WHEN: return "'when'";
            case TokenType::KW_IF: return "'if'";
            case TokenType::KW_THEN: return "'then'";
            case TokenType::KW_OTHERWISE: return "'otherwise'";
            case TokenType::KW_REPEAT: return "'repeat'";
            case TokenType::KW_TIMES: return "'times'";
            case TokenType::KW_ADD: return "'add'";
            case TokenType::KW_TO: return "'to'";
            case TokenType::KW_SUBTRACT: return "'subtract'";
            case TokenType::KW_SHOW: return "'show'";
            case TokenType::KW_PLAY: return "'play'";
            case TokenType::KW_HIDE: return "'hide'";
            case TokenType::KW_DISPLAY: return "'display'";
            case TokenType::KW_AT: return "'at'";
            case TokenType::KW_EQUAL: return "'equal'";
            case TokenType::KW_GREATER: return "'greater'";
            case TokenType::KW_LESS: return "'less'";
            case TokenType::KW_THAN: return "'than'";
            case TokenType::KW_NOT: return "'not'";
            case TokenType::KW_PLUS: return "'plus'";
            case TokenType::KW_MINUS: return "'minus'";
            case TokenType::KW_DIVIDED: return "'divided'";
            case TokenType::KW_BY: return "'by'";
            case TokenType::KW_TRUE: return "'true'";
            case TokenType::KW_FALSE: return "'false'";
        }
        return "token";
    }
};

} // namespace lore

#endif // LORE_TOKEN_H
