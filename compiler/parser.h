#ifndef LORE_PARSER_H
#define LORE_PARSER_H

#include "token.h"
#include "ast.h"
#include "source_pos.h"
#include <vector>
#include <memory>

namespace lore {

class Parser {
public:
    Parser(const std::string& filename, const std::vector<Token>& tokens);

    std::shared_ptr<ProgramNode> parse_program();

private:
    std::string filename_;
    std::vector<Token> tokens_;
    size_t cursor_ = 0;

    const Token& peek(size_t offset = 0) const;
    const Token& advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    const Token& expect(TokenType type, const std::string& expected_msg);
    bool is_at_end() const;

    // Grammar parsers
    std::shared_ptr<PackageHeaderNode> parse_package_header();
    std::shared_ptr<ImportNode> parse_import();
    std::shared_ptr<PermissionBlockNode> parse_permission_block();
    std::shared_ptr<HCCSBlockNode> parse_hccs_block();
    std::shared_ptr<RootDeclNode> parse_root_decl();
    std::shared_ptr<ASTNode> parse_top_item();

    std::shared_ptr<ElementDeclNode> parse_element_decl();
    std::shared_ptr<AttrStmtNode> parse_attr_stmt();
    std::shared_ptr<AttrEndNode> parse_attr_end();
    std::shared_ptr<StateDeclNode> parse_state_decl();
    std::shared_ptr<HandlerNode> parse_handler();

    std::shared_ptr<StatementNode> parse_statement();
    std::shared_ptr<AssignStmtNode> parse_assign_stmt();
    std::shared_ptr<IfStmtNode> parse_if_stmt();
    std::shared_ptr<RepeatStmtNode> parse_repeat_stmt();
    std::shared_ptr<CallStmtNode> parse_call_stmt();

    std::shared_ptr<ConditionNode> parse_condition();
    Comparator parse_comparator();

    std::shared_ptr<ExprNode> parse_expr();
    std::shared_ptr<TermNode> parse_term();
    std::shared_ptr<FactorNode> parse_factor();

    bool is_element_kind(const std::string& s) const;
    bool is_attr_keyword(const std::string& s) const;
};

} // namespace lore

#endif // LORE_PARSER_H
