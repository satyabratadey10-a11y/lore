#ifndef LORE_CODEGEN_CPP_H
#define LORE_CODEGEN_CPP_H

#include "ast.h"
#include <string>
#include <sstream>
#include <memory>
#include <unordered_map>

namespace lore {

class CodegenCpp {
public:
    CodegenCpp();

    std::string generate(const std::shared_ptr<ProgramNode>& program);

private:
    std::ostringstream out_;
    int indent_level_ = 0;
    std::unordered_map<std::string, std::string> state_types_; // name -> "int" or "string"

    void emit_indent();
    void emit_line(const std::string& line);

    void gen_header();
    void gen_package(const std::shared_ptr<PackageHeaderNode>& header);
    void gen_permissions(const std::shared_ptr<PermissionBlockNode>& perms);
    void gen_hccs(const std::shared_ptr<HCCSBlockNode>& hccs);
    void gen_root(const std::shared_ptr<RootDeclNode>& root);
    void gen_top_item(const std::shared_ptr<ASTNode>& item);

    void gen_element_decl(const std::shared_ptr<ElementDeclNode>& el);
    void gen_attr_stmt(const std::shared_ptr<AttrStmtNode>& attr);
    void gen_state_decl(const std::shared_ptr<StateDeclNode>& state);
    void gen_handler(const std::shared_ptr<HandlerNode>& handler);

    void gen_statement(const std::shared_ptr<StatementNode>& stmt);
    void gen_assign_stmt(const std::shared_ptr<AssignStmtNode>& assign);
    void gen_if_stmt(const std::shared_ptr<IfStmtNode>& if_stmt);
    void gen_repeat_stmt(const std::shared_ptr<RepeatStmtNode>& repeat);
    void gen_call_stmt(const std::shared_ptr<CallStmtNode>& call);

    std::string gen_condition(const std::shared_ptr<ConditionNode>& cond);
    std::string gen_expr(const std::shared_ptr<ExprNode>& expr);
    std::string gen_term(const std::shared_ptr<TermNode>& term);
    std::string gen_factor(const std::shared_ptr<FactorNode>& factor);

    std::string map_element_kind(const std::string& kind);
    std::string map_image_fit(const std::string& fit);
    std::string map_align(const std::string& align);
    std::string map_unit(const std::string& unit);
    std::string map_event(const std::string& event);
};

} // namespace lore

#endif // LORE_CODEGEN_CPP_H
