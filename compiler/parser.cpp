#include "parser.h"
#include <algorithm>

namespace lore {

Parser::Parser(const std::string& filename, const std::vector<Token>& tokens)
    : filename_(filename), tokens_(tokens) {}

const Token& Parser::peek(size_t offset) const {
    if (cursor_ + offset >= tokens_.size()) return tokens_.back();
    return tokens_[cursor_ + offset];
}

const Token& Parser::advance() {
    if (!is_at_end()) cursor_++;
    return tokens_[cursor_ - 1];
}

bool Parser::check(TokenType type) const {
    if (is_at_end()) return type == TokenType::END_OF_FILE;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

const Token& Parser::expect(TokenType type, const std::string& expected_msg) {
    if (check(type)) {
        return advance();
    }
    const Token& cur = peek();
    throw CompileError(cur.pos, "I expected " + expected_msg + " but found " + cur.type_name() + ".");
}

bool Parser::is_at_end() const {
    return cursor_ >= tokens_.size() || tokens_[cursor_].type == TokenType::END_OF_FILE;
}

bool Parser::is_element_kind(const std::string& s) const {
    return (s == "bg" || s == "text" || s == "button" || s == "image" ||
            s == "row" || s == "column" || s == "box");
}

bool Parser::is_attr_keyword(const std::string& s) const {
    return (s == "fullscreen" || s == "cover" || s == "contain" || s == "stretch" ||
            s == "tile" || s == "center" || s == "start" || s == "end" || s == "wrap");
}

std::shared_ptr<ProgramNode> Parser::parse_program() {
    auto prog = std::make_shared<ProgramNode>();
    prog->pos = peek().pos;

    // 1. Package header
    prog->header = parse_package_header();

    // 2. Imports (0 or more)
    while (check(TokenType::KW_FROM) || check(TokenType::KW_IMPORT)) {
        prog->imports.push_back(parse_import());
    }

    // 3. Permission block (optional)
    if (check(TokenType::KW_PERMISSION)) {
        prog->permissions = parse_permission_block();
    }

    // 4. HCCS block (optional)
    if (check(TokenType::KW_HCCS)) {
        prog->hccs = parse_hccs_block();
    }

    // 5. Root declaration
    prog->root = parse_root_decl();

    // 6. Top items until EOF
    while (!is_at_end()) {
        auto item = parse_top_item();
        if (item) {
            prog->top_items.push_back(item);
        }
    }

    return prog;
}

std::shared_ptr<PackageHeaderNode> Parser::parse_package_header() {
    auto header = std::make_shared<PackageHeaderNode>();
    header->pos = peek().pos;

    int fields_found = 0;
    while (fields_found < 3 && !is_at_end()) {
        expect(TokenType::KW_PACKAGE_COLON, "\"package:\"");
        const Token& key_tok = expect(TokenType::IDENT, "package field ('name', 'id', or 'label')");
        std::string key = key_tok.text;
        expect(TokenType::COLON, "':'");
        const Token& val_tok = expect(TokenType::STRING, "package string value");
        expect(TokenType::DOT, "'.' to terminate package field");

        if (key == "name") header->name = val_tok.text;
        else if (key == "id") header->id = val_tok.text;
        else if (key == "label" || key == "lable") header->label = val_tok.text;
        else {
            throw CompileError(key_tok.pos, "Unknown package field '" + key + "'. Expected 'name', 'id', or 'label'.");
        }
        fields_found++;
    }

    if (header->name.empty() || header->id.empty() || header->label.empty()) {
        throw CompileError(header->pos, "Package header must define 'name', 'id', and 'label'.");
    }
    return header;
}

std::shared_ptr<ImportNode> Parser::parse_import() {
    auto node = std::make_shared<ImportNode>();
    node->pos = peek().pos;

    if (match(TokenType::KW_FROM)) {
        expect(TokenType::LBRACKET, "'[' after from");
        const Token& m = expect(TokenType::IDENT, "module name");
        node->from_module = m.text;
        expect(TokenType::RBRACKET, "']' after module name");
    }

    expect(TokenType::KW_IMPORT, "'import'");
    expect(TokenType::LBRACKET, "'[' after import");
    const Token& item_tok = expect(TokenType::IDENT, "imported item name");
    node->item = item_tok.text;
    expect(TokenType::RBRACKET, "']' after item name");
    expect(TokenType::DOT, "'.' to terminate import statement");

    return node;
}

std::shared_ptr<PermissionBlockNode> Parser::parse_permission_block() {
    auto block = std::make_shared<PermissionBlockNode>();
    block->pos = peek().pos;
    expect(TokenType::KW_PERMISSION, "'permission'");

    while (!check(TokenType::END_OF_PERMISSION) && !is_at_end()) {
        if (check(TokenType::KW_ROOT) || check(TokenType::KW_HCCS) ||
            check(TokenType::KW_PUT) || check(TokenType::KW_STATE) ||
            check(TokenType::KW_WHEN) ||
            (check(TokenType::IDENT) && peek(1).type != TokenType::COLON)) {
            throw CompileError(peek().pos, "I expected \"EndOfPermission.\" to close the permission block.");
        }

        bool more_items = true;
        while (more_items) {
            auto item = std::make_shared<PermItemNode>();
            item->pos = peek().pos;
            const Token& name_tok = expect(TokenType::IDENT, "permission name");
            item->name = name_tok.text;
            expect(TokenType::COLON, "':'");

            if (check(TokenType::KW_TRUE)) {
                advance();
                item->is_bool = true;
                item->bool_value = true;
            } else if (check(TokenType::KW_FALSE)) {
                advance();
                item->is_bool = true;
                item->bool_value = false;
            } else if (check(TokenType::NUMBER)) {
                const Token& num = advance();
                item->is_bool = false;
                item->num_value = num.num_value;
            } else {
                throw CompileError(peek().pos, "I expected a permission value ('true', 'false', or a number).");
            }
            block->items.push_back(item);

            if (match(TokenType::KW_AND)) {
                more_items = true;
            } else {
                more_items = false;
            }
        }
        expect(TokenType::DOT, "'.' to terminate permission line");
    }

    expect(TokenType::END_OF_PERMISSION, "\"EndOfPermission.\" to close the permission block");
    return block;
}

std::shared_ptr<HCCSBlockNode> Parser::parse_hccs_block() {
    auto block = std::make_shared<HCCSBlockNode>();
    block->pos = peek().pos;
    expect(TokenType::KW_HCCS, "'HCCS'");

    while (!check(TokenType::END_OF_HCCS) && !is_at_end()) {
        auto item = std::make_shared<HCCSItemNode>();
        item->pos = peek().pos;
        const Token& hex_tok = expect(TokenType::STRING, "hex colour string (e.g. \"#4287f5\")");
        item->hex_code = hex_tok.text;
        expect(TokenType::KW_IS, "'is'");
        const Token& name_tok = expect(TokenType::STRING, "colour name string (e.g. \"blue\")");
        item->name = name_tok.text;
        expect(TokenType::DOT, "'.' to terminate HCCS entry");
        block->items.push_back(item);
    }

    expect(TokenType::END_OF_HCCS, "\"EndOfHCCS.\" to close the HCCS block");
    return block;
}

std::shared_ptr<RootDeclNode> Parser::parse_root_decl() {
    auto node = std::make_shared<RootDeclNode>();
    node->pos = peek().pos;
    expect(TokenType::KW_ROOT, "'root'");
    expect(TokenType::KW_IS, "'is'");

    const Token& kind_tok = expect(TokenType::IDENT, "element kind ('bg', 'box', 'row', 'column', etc.)");
    node->kind = kind_tok.text;
    node->handle = kind_tok.text; // root handle defaults to kind e.g. "bg"
    expect(TokenType::DOT, "'.' to terminate root declaration");

    return node;
}

std::shared_ptr<ASTNode> Parser::parse_top_item() {
    if (check(TokenType::KW_PUT)) {
        return parse_element_decl();
    }
    if (check(TokenType::ATTR_END)) {
        return parse_attr_end();
    }
    if (check(TokenType::KW_STATE)) {
        return parse_state_decl();
    }
    if (check(TokenType::KW_WHEN)) {
        return parse_handler();
    }
    if (check(TokenType::KW_SHOW) || check(TokenType::KW_HIDE)) {
        return parse_call_stmt();
    }
    if (check(TokenType::IDENT)) {
        return parse_attr_stmt();
    }

    const Token& cur = peek();
    throw CompileError(cur.pos, "Unexpected token in top-level item: " + cur.type_name());
}

std::shared_ptr<ElementDeclNode> Parser::parse_element_decl() {
    auto node = std::make_shared<ElementDeclNode>();
    node->pos = peek().pos;
    expect(TokenType::KW_PUT, "'put'");

    const Token& kind_tok = expect(TokenType::IDENT, "element kind ('bg', 'text', 'button', 'image', 'row', 'column', 'box')");
    node->kind = kind_tok.text;

    const Token& handle_tok = expect(TokenType::IDENT, "element handle name");
    node->handle = handle_tok.text;

    expect(TokenType::KW_INSIDE, "'inside'");

    const Token& parent_tok = expect(TokenType::IDENT, "parent handle name");
    node->parent_handle = parent_tok.text;

    expect(TokenType::DOT, "'.' to terminate element declaration");
    return node;
}

std::shared_ptr<AttrStmtNode> Parser::parse_attr_stmt() {
    auto node = std::make_shared<AttrStmtNode>();
    node->pos = peek().pos;

    const Token& handle_tok = expect(TokenType::IDENT, "element handle");
    node->handle = handle_tok.text;

    // Attribute name: ident or ident:ident
    const Token& attr_tok = expect(TokenType::IDENT, "attribute name");
    std::string attr = attr_tok.text;
    if (match(TokenType::COLON)) {
        const Token& sub = expect(TokenType::IDENT, "attribute sub-property");
        attr += ":" + sub.text;
    }
    node->attr_name = attr;

    // Optional "is"
    match(TokenType::KW_IS);

    // Value: keyword (fullscreen, cover, etc.), factor, or expression
    if (check(TokenType::IDENT) && is_attr_keyword(peek().text)) {
        node->keyword_val = advance().text;
    } else {
        node->expr_val = parse_expr();
    }

    expect(TokenType::DOT, "'.' to terminate attribute line");
    return node;
}

std::shared_ptr<AttrEndNode> Parser::parse_attr_end() {
    auto node = std::make_shared<AttrEndNode>();
    node->pos = peek().pos;
    const Token& tok = expect(TokenType::ATTR_END, "'.end<handle>att.'");
    node->handle = tok.handle;
    return node;
}

std::shared_ptr<StateDeclNode> Parser::parse_state_decl() {
    auto node = std::make_shared<StateDeclNode>();
    node->pos = peek().pos;
    expect(TokenType::KW_STATE, "'state'");
    const Token& name_tok = expect(TokenType::IDENT, "state variable name");
    node->name = name_tok.text;
    expect(TokenType::KW_IS, "'is'");
    node->expr = parse_expr();
    expect(TokenType::DOT, "'.' to terminate state declaration");
    return node;
}

std::shared_ptr<HandlerNode> Parser::parse_handler() {
    auto node = std::make_shared<HandlerNode>();
    node->pos = peek().pos;
    expect(TokenType::KW_WHEN, "'when'");
    const Token& handle_tok = expect(TokenType::IDENT, "element handle");
    node->handle = handle_tok.text;
    expect(TokenType::KW_IS, "'is'");

    const Token& ev_tok = expect(TokenType::IDENT, "event name ('tapped', 'pressed', 'released', 'changed', 'input')");
    node->event_name = ev_tok.text;

    while (!check(TokenType::END_OF_WHEN) && !is_at_end()) {
        if (check(TokenType::KW_WHEN) || check(TokenType::KW_STATE) || check(TokenType::KW_PUT) || check(TokenType::KW_ROOT)) {
            throw CompileError(peek().pos, "I expected \"EndOfWhen.\" to close the when block.");
        }
        node->statements.push_back(parse_statement());
    }

    expect(TokenType::END_OF_WHEN, "\"EndOfWhen.\" to close the when block");
    return node;
}

std::shared_ptr<StatementNode> Parser::parse_statement() {
    if (check(TokenType::KW_IF)) {
        return parse_if_stmt();
    }
    if (check(TokenType::KW_REPEAT)) {
        return parse_repeat_stmt();
    }
    if (check(TokenType::KW_ADD) || check(TokenType::KW_SUBTRACT) ||
        check(TokenType::KW_SHOW) || check(TokenType::KW_PLAY) ||
        check(TokenType::KW_HIDE)) {
        return parse_call_stmt();
    }
    if (check(TokenType::IDENT)) {
        return parse_assign_stmt();
    }

    throw CompileError(peek().pos, "Unexpected statement starting with " + peek().type_name());
}

std::shared_ptr<AssignStmtNode> Parser::parse_assign_stmt() {
    auto node = std::make_shared<AssignStmtNode>();
    node->pos = peek().pos;
    const Token& t = expect(TokenType::IDENT, "variable name");
    node->target = t.text;
    expect(TokenType::KW_IS, "'is'");
    node->expr = parse_expr();
    expect(TokenType::DOT, "'.' to terminate assignment statement");
    return node;
}

std::shared_ptr<IfStmtNode> Parser::parse_if_stmt() {
    auto node = std::make_shared<IfStmtNode>();
    node->pos = peek().pos;
    expect(TokenType::KW_IF, "'if'");
    node->condition = parse_condition();
    expect(TokenType::KW_THEN, "'then'");

    while (!check(TokenType::END_OF_IF) && !check(TokenType::KW_OTHERWISE) && !is_at_end()) {
        if (check(TokenType::END_OF_WHEN) || check(TokenType::KW_WHEN) || check(TokenType::KW_STATE)) {
            throw CompileError(peek().pos, "I expected \"EndOfIf.\" to close the if block.");
        }
        node->then_branch.push_back(parse_statement());
    }

    if (match(TokenType::KW_OTHERWISE)) {
        while (!check(TokenType::END_OF_IF) && !is_at_end()) {
            if (check(TokenType::END_OF_WHEN) || check(TokenType::KW_WHEN) || check(TokenType::KW_STATE)) {
                throw CompileError(peek().pos, "I expected \"EndOfIf.\" to close the if block.");
            }
            node->otherwise_branch.push_back(parse_statement());
        }
    }

    expect(TokenType::END_OF_IF, "\"EndOfIf.\" to close the if block");
    return node;
}

std::shared_ptr<RepeatStmtNode> Parser::parse_repeat_stmt() {
    auto node = std::make_shared<RepeatStmtNode>();
    node->pos = peek().pos;
    expect(TokenType::KW_REPEAT, "'repeat'");
    node->count_expr = parse_expr();
    expect(TokenType::KW_TIMES, "'times'");

    while (!check(TokenType::END_OF_REPEAT) && !is_at_end()) {
        if (check(TokenType::END_OF_WHEN) || check(TokenType::KW_WHEN) || check(TokenType::KW_STATE)) {
            throw CompileError(peek().pos, "I expected \"EndOfRepeat.\" to close the repeat block.");
        }
        node->body.push_back(parse_statement());
    }

    expect(TokenType::END_OF_REPEAT, "\"EndOfRepeat.\" to close the repeat block");
    return node;
}

std::shared_ptr<CallStmtNode> Parser::parse_call_stmt() {
    auto node = std::make_shared<CallStmtNode>();
    node->pos = peek().pos;

    if (match(TokenType::KW_ADD)) {
        node->kind = CallKind::ADD;
        node->expr = parse_expr();
        expect(TokenType::KW_TO, "'to'");
        const Token& tgt = expect(TokenType::IDENT, "target identifier");
        node->target = tgt.text;
        expect(TokenType::DOT, "'.' to terminate add statement");
    } else if (match(TokenType::KW_SUBTRACT)) {
        node->kind = CallKind::SUBTRACT;
        node->expr = parse_expr();
        expect(TokenType::KW_FROM, "'from'");
        const Token& tgt = expect(TokenType::IDENT, "target identifier");
        node->target = tgt.text;
        expect(TokenType::DOT, "'.' to terminate subtract statement");
    } else if (match(TokenType::KW_SHOW)) {
        // Could be "show display at input." or "show <expr>."
        if (check(TokenType::KW_DISPLAY)) {
            advance();
            expect(TokenType::KW_AT, "'at'");
            const Token& stage_tok = expect(TokenType::IDENT, "stage ('input', 'process', or 'output')");
            node->kind = CallKind::SHOW_DISPLAY_AT;
            node->target = stage_tok.text;
            expect(TokenType::DOT, "'.' to terminate show display statement");
        } else {
            node->kind = CallKind::SHOW;
            node->expr = parse_expr();
            expect(TokenType::DOT, "'.' to terminate show statement");
        }
    } else if (match(TokenType::KW_HIDE)) {
        expect(TokenType::KW_DISPLAY, "'display'");
        node->kind = CallKind::HIDE_DISPLAY;
        expect(TokenType::DOT, "'.' to terminate hide display statement");
    } else if (match(TokenType::KW_PLAY)) {
        node->kind = CallKind::PLAY;
        const Token& sound_tok = expect(TokenType::IDENT, "sound identifier");
        node->target = sound_tok.text;
        expect(TokenType::DOT, "'.' to terminate play statement");
    } else {
        throw CompileError(peek().pos, "Unknown call statement");
    }

    return node;
}

Comparator Parser::parse_comparator() {
    expect(TokenType::KW_IS, "'is'");
    if (match(TokenType::KW_EQUAL)) {
        expect(TokenType::KW_TO, "'to'");
        return Comparator::IS_EQUAL_TO;
    }
    if (match(TokenType::KW_GREATER)) {
        expect(TokenType::KW_THAN, "'than'");
        return Comparator::IS_GREATER_THAN;
    }
    if (match(TokenType::KW_LESS)) {
        expect(TokenType::KW_THAN, "'than'");
        return Comparator::IS_LESS_THAN;
    }
    if (match(TokenType::KW_NOT)) {
        return Comparator::IS_NOT;
    }

    throw CompileError(peek().pos, "I expected a comparison operator ('is equal to', 'is greater than', 'is less than', or 'is not').");
}

std::shared_ptr<ConditionNode> Parser::parse_condition() {
    auto node = std::make_shared<ConditionNode>();
    node->pos = peek().pos;

    node->left = parse_expr();
    node->comp = parse_comparator();
    node->right = parse_expr();

    if (match(TokenType::KW_AND)) {
        node->logical_op = LogicalOp::AND;
        node->next = parse_condition();
    } else if (match(TokenType::KW_OR)) {
        node->logical_op = LogicalOp::OR;
        node->next = parse_condition();
    }

    return node;
}

std::shared_ptr<ExprNode> Parser::parse_expr() {
    auto node = std::make_shared<ExprNode>();
    node->pos = peek().pos;

    node->first = parse_term();

    while (check(TokenType::KW_PLUS) || check(TokenType::KW_MINUS)) {
        ExprItem item;
        if (match(TokenType::KW_PLUS)) item.op = BinaryOp::PLUS;
        else if (match(TokenType::KW_MINUS)) item.op = BinaryOp::MINUS;
        item.term = parse_term();
        node->rest.push_back(item);
    }

    return node;
}

std::shared_ptr<TermNode> Parser::parse_term() {
    auto node = std::make_shared<TermNode>();
    node->pos = peek().pos;

    node->first = parse_factor();

    while (check(TokenType::KW_TIMES) || check(TokenType::KW_DIVIDED)) {
        TermItem item;
        if (match(TokenType::KW_TIMES)) {
            item.op = BinaryOp::TIMES;
        } else if (match(TokenType::KW_DIVIDED)) {
            expect(TokenType::KW_BY, "'by' after divided");
            item.op = BinaryOp::DIVIDED_BY;
        }
        item.factor = parse_factor();
        node->rest.push_back(item);
    }

    return node;
}

std::shared_ptr<FactorNode> Parser::parse_factor() {
    auto node = std::make_shared<FactorNode>();
    node->pos = peek().pos;

    if (check(TokenType::NUMBER)) {
        const Token& t = advance();
        node->kind = FactorKind::NUMBER;
        node->num_val = t.num_value;
        node->text = t.text;
        return node;
    }
    if (check(TokenType::DIMENSION)) {
        const Token& t = advance();
        node->kind = FactorKind::DIMENSION;
        node->num_val = t.num_value;
        node->unit = t.unit;
        node->text = t.text;
        return node;
    }
    if (check(TokenType::STRING)) {
        const Token& t = advance();
        node->kind = FactorKind::STRING;
        node->text = t.text;
        return node;
    }
    if (check(TokenType::HEX_COLOR)) {
        const Token& t = advance();
        node->kind = FactorKind::HEX_COLOR;
        node->text = t.text;
        return node;
    }
    if (check(TokenType::COLOR_REF)) {
        const Token& t = advance();
        node->kind = FactorKind::COLOR_REF;
        node->text = t.text;
        return node;
    }
    if (check(TokenType::IDENT)) {
        const Token& t = advance();
        node->kind = FactorKind::IDENT;
        node->text = t.text;
        return node;
    }
    if (match(TokenType::LPAREN)) {
        node->kind = FactorKind::GROUPED;
        node->grouped_expr = parse_expr();
        expect(TokenType::RPAREN, "')'");
        return node;
    }

    throw CompileError(peek().pos, "I expected a value or expression factor, but found " + peek().type_name() + ".");
}

} // namespace lore
