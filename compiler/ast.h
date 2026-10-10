#ifndef LORE_AST_H
#define LORE_AST_H

#include "source_pos.h"
#include <string>
#include <vector>
#include <memory>
#include <iostream>

namespace lore {

class ASTVisitor;

struct ASTNode {
    SourcePos pos;
    virtual ~ASTNode() = default;
    virtual void dump(std::ostream& os, int indent = 0) const = 0;
};

inline void print_indent(std::ostream& os, int indent) {
    for (int i = 0; i < indent; ++i) os << "  ";
}

// ============================================================================
// Expressions
// ============================================================================

enum class FactorKind {
    NUMBER,
    DIMENSION,
    STRING,
    HEX_COLOR,
    COLOR_REF,
    IDENT,
    GROUPED
};

struct FactorNode : public ASTNode {
    FactorKind kind = FactorKind::NUMBER;
    std::string text;
    double num_val = 0.0;
    std::string unit;
    std::shared_ptr<struct ExprNode> grouped_expr;

    void dump(std::ostream& os, int indent = 0) const override;
};

enum class BinaryOp {
    NONE,
    PLUS,
    MINUS,
    TIMES,
    DIVIDED_BY
};

inline std::string binary_op_name(BinaryOp op) {
    switch (op) {
        case BinaryOp::PLUS: return "plus";
        case BinaryOp::MINUS: return "minus";
        case BinaryOp::TIMES: return "times";
        case BinaryOp::DIVIDED_BY: return "divided by";
        default: return "";
    }
}

struct TermItem {
    BinaryOp op = BinaryOp::NONE; // TIMES or DIVIDED_BY
    std::shared_ptr<FactorNode> factor;
};

struct TermNode : public ASTNode {
    std::shared_ptr<FactorNode> first;
    std::vector<TermItem> rest;

    void dump(std::ostream& os, int indent = 0) const override;
};

struct ExprItem {
    BinaryOp op = BinaryOp::NONE; // PLUS or MINUS
    std::shared_ptr<TermNode> term;
};

struct ExprNode : public ASTNode {
    std::shared_ptr<TermNode> first;
    std::vector<ExprItem> rest;

    void dump(std::ostream& os, int indent = 0) const override;
};

// ============================================================================
// Conditions
// ============================================================================

enum class Comparator {
    IS_EQUAL_TO,
    IS_GREATER_THAN,
    IS_LESS_THAN,
    IS_NOT
};

inline std::string comparator_name(Comparator c) {
    switch (c) {
        case Comparator::IS_EQUAL_TO: return "is equal to";
        case Comparator::IS_GREATER_THAN: return "is greater than";
        case Comparator::IS_LESS_THAN: return "is less than";
        case Comparator::IS_NOT: return "is not";
    }
    return "";
}

enum class LogicalOp {
    NONE,
    AND,
    OR
};

struct ConditionNode : public ASTNode {
    std::shared_ptr<ExprNode> left;
    Comparator comp = Comparator::IS_EQUAL_TO;
    std::shared_ptr<ExprNode> right;

    LogicalOp logical_op = LogicalOp::NONE;
    std::shared_ptr<ConditionNode> next;

    void dump(std::ostream& os, int indent = 0) const override;
};

// ============================================================================
// Statements
// ============================================================================

struct StatementNode : public ASTNode {};

struct AssignStmtNode : public StatementNode {
    std::string target;
    std::shared_ptr<ExprNode> expr;

    void dump(std::ostream& os, int indent = 0) const override;
};

struct IfStmtNode : public StatementNode {
    std::shared_ptr<ConditionNode> condition;
    std::vector<std::shared_ptr<StatementNode>> then_branch;
    std::vector<std::shared_ptr<StatementNode>> otherwise_branch;

    void dump(std::ostream& os, int indent = 0) const override;
};

struct RepeatStmtNode : public StatementNode {
    std::shared_ptr<ExprNode> count_expr;
    std::vector<std::shared_ptr<StatementNode>> body;

    void dump(std::ostream& os, int indent = 0) const override;
};

enum class CallKind {
    ADD,
    SUBTRACT,
    SHOW,
    PLAY,
    SHOW_DISPLAY_AT,
    HIDE_DISPLAY
};

struct CallStmtNode : public StatementNode {
    CallKind kind = CallKind::ADD;
    std::shared_ptr<ExprNode> expr;
    std::string target; // For ADD to / SUBTRACT from / PLAY ident / stage name

    void dump(std::ostream& os, int indent = 0) const override;
};

// ============================================================================
// UI Elements & Attributes
// ============================================================================

struct PackageFieldNode : public ASTNode {
    std::string key; // "name", "id", "label"
    std::string value;

    void dump(std::ostream& os, int indent = 0) const override;
};

struct PackageHeaderNode : public ASTNode {
    std::string name;
    std::string id;
    std::string label;

    void dump(std::ostream& os, int indent = 0) const override;
};

struct ImportNode : public ASTNode {
    std::string from_module;
    std::string item;

    void dump(std::ostream& os, int indent = 0) const override;
};

struct PermItemNode : public ASTNode {
    std::string name;
    bool is_bool = true;
    bool bool_value = false;
    double num_value = 0.0;

    void dump(std::ostream& os, int indent = 0) const override;
};

struct PermissionBlockNode : public ASTNode {
    std::vector<std::shared_ptr<PermItemNode>> items;

    void dump(std::ostream& os, int indent = 0) const override;
};

struct HCCSItemNode : public ASTNode {
    std::string hex_code;
    std::string name;

    void dump(std::ostream& os, int indent = 0) const override;
};

struct HCCSBlockNode : public ASTNode {
    std::vector<std::shared_ptr<HCCSItemNode>> items;

    void dump(std::ostream& os, int indent = 0) const override;
};

struct RootDeclNode : public ASTNode {
    std::string kind;
    std::string handle = "bg";

    void dump(std::ostream& os, int indent = 0) const override;
};

struct ElementDeclNode : public ASTNode {
    std::string kind;
    std::string handle;
    std::string parent_handle;

    void dump(std::ostream& os, int indent = 0) const override;
};

struct AttrStmtNode : public ASTNode {
    std::string handle;
    std::string attr_name; // e.g. "size", "image", "image:pressed", "image-fit"
    std::shared_ptr<FactorNode> factor_val;
    std::shared_ptr<ExprNode> expr_val;
    std::string keyword_val;

    void dump(std::ostream& os, int indent = 0) const override;
};

struct AttrEndNode : public ASTNode {
    std::string handle;

    void dump(std::ostream& os, int indent = 0) const override;
};

struct StateDeclNode : public ASTNode {
    std::string name;
    std::shared_ptr<ExprNode> expr;

    void dump(std::ostream& os, int indent = 0) const override;
};

struct HandlerNode : public ASTNode {
    std::string handle;
    std::string event_name; // "tapped", "pressed", "released", "changed", "input"
    std::vector<std::shared_ptr<StatementNode>> statements;

    void dump(std::ostream& os, int indent = 0) const override;
};

// ============================================================================
// Program Root Node
// ============================================================================

struct ProgramNode : public ASTNode {
    std::shared_ptr<PackageHeaderNode> header;
    std::vector<std::shared_ptr<ImportNode>> imports;
    std::shared_ptr<PermissionBlockNode> permissions;
    std::shared_ptr<HCCSBlockNode> hccs;
    std::shared_ptr<RootDeclNode> root;

    // Top-level items in order of occurrence
    std::vector<std::shared_ptr<ASTNode>> top_items;

    void dump(std::ostream& os, int indent = 0) const override;
};

} // namespace lore

#endif // LORE_AST_H
