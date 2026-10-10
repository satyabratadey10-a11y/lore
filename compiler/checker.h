#ifndef LORE_CHECKER_H
#define LORE_CHECKER_H

#include "ast.h"
#include "source_pos.h"
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <memory>

namespace lore {

enum class StateType {
    INT,
    STRING,
    UNKNOWN
};

class Checker {
public:
    explicit Checker(const std::string& base_dir = ".");

    void check_program(const std::shared_ptr<ProgramNode>& program);

    const std::vector<std::string>& warnings() const { return warnings_; }

private:
    std::string base_dir_;
    std::vector<std::string> warnings_;

    std::unordered_set<std::string> hccs_colors_;
    std::unordered_map<std::string, std::string> element_kinds_; // handle -> kind
    std::unordered_map<std::string, StateType> states_;          // name -> type

    std::string current_attr_handle_;

    void check_permissions(const std::shared_ptr<PermissionBlockNode>& permissions);
    void check_hccs(const std::shared_ptr<HCCSBlockNode>& hccs);
    void check_root(const std::shared_ptr<RootDeclNode>& root);
    void check_top_items(const std::vector<std::shared_ptr<ASTNode>>& items);

    void check_element_decl(const std::shared_ptr<ElementDeclNode>& el);
    void check_attr_stmt(const std::shared_ptr<AttrStmtNode>& attr);
    void check_attr_end(const std::shared_ptr<AttrEndNode>& end);
    void check_state_decl(const std::shared_ptr<StateDeclNode>& state);
    void check_handler(const std::shared_ptr<HandlerNode>& handler);

    void check_statement(const std::shared_ptr<StatementNode>& stmt);
    void check_assign_stmt(const std::shared_ptr<AssignStmtNode>& assign);
    void check_if_stmt(const std::shared_ptr<IfStmtNode>& if_stmt);
    void check_repeat_stmt(const std::shared_ptr<RepeatStmtNode>& repeat);
    void check_call_stmt(const std::shared_ptr<CallStmtNode>& call);

    void check_condition(const std::shared_ptr<ConditionNode>& cond);
    StateType check_expr(const std::shared_ptr<ExprNode>& expr);
    StateType check_term(const std::shared_ptr<TermNode>& term);
    StateType check_factor(const std::shared_ptr<FactorNode>& factor);

    void check_image_path(const SourcePos& pos, const std::string& path);
};

} // namespace lore

#endif // LORE_CHECKER_H
