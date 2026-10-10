#include "ast.h"

namespace lore {

void FactorNode::dump(std::ostream& os, int indent) const {
    print_indent(os, indent);
    switch (kind) {
        case FactorKind::NUMBER:
            os << "Number(" << num_val << ")\n";
            break;
        case FactorKind::DIMENSION:
            os << "Dimension(" << num_val << unit << ")\n";
            break;
        case FactorKind::STRING:
            os << "String(\"" << text << "\")\n";
            break;
        case FactorKind::HEX_COLOR:
            os << "HexColor(" << text << ")\n";
            break;
        case FactorKind::COLOR_REF:
            os << "ColorRef(" << text << ")\n";
            break;
        case FactorKind::IDENT:
            os << "Ident(" << text << ")\n";
            break;
        case FactorKind::GROUPED:
            os << "GroupedExpr:\n";
            if (grouped_expr) grouped_expr->dump(os, indent + 1);
            break;
    }
}

void TermNode::dump(std::ostream& os, int indent) const {
    print_indent(os, indent);
    os << "Term:\n";
    if (first) first->dump(os, indent + 1);
    for (const auto& item : rest) {
        print_indent(os, indent + 1);
        os << "Op(" << binary_op_name(item.op) << "):\n";
        if (item.factor) item.factor->dump(os, indent + 2);
    }
}

void ExprNode::dump(std::ostream& os, int indent) const {
    print_indent(os, indent);
    os << "Expr:\n";
    if (first) first->dump(os, indent + 1);
    for (const auto& item : rest) {
        print_indent(os, indent + 1);
        os << "Op(" << binary_op_name(item.op) << "):\n";
        if (item.term) item.term->dump(os, indent + 2);
    }
}

void ConditionNode::dump(std::ostream& os, int indent) const {
    print_indent(os, indent);
    os << "Condition(" << comparator_name(comp) << "):\n";
    print_indent(os, indent + 1);
    os << "Left:\n";
    if (left) left->dump(os, indent + 2);
    print_indent(os, indent + 1);
    os << "Right:\n";
    if (right) right->dump(os, indent + 2);
    if (logical_op != LogicalOp::NONE && next) {
        print_indent(os, indent + 1);
        os << (logical_op == LogicalOp::AND ? "AND" : "OR") << ":\n";
        next->dump(os, indent + 2);
    }
}

void AssignStmtNode::dump(std::ostream& os, int indent) const {
    print_indent(os, indent);
    os << "AssignStmt(target=" << target << "):\n";
    if (expr) expr->dump(os, indent + 1);
}

void IfStmtNode::dump(std::ostream& os, int indent) const {
    print_indent(os, indent);
    os << "IfStmt:\n";
    if (condition) condition->dump(os, indent + 1);
    print_indent(os, indent);
    os << "Then:\n";
    for (const auto& stmt : then_branch) stmt->dump(os, indent + 1);
    if (!otherwise_branch.empty()) {
        print_indent(os, indent);
        os << "Otherwise:\n";
        for (const auto& stmt : otherwise_branch) stmt->dump(os, indent + 1);
    }
}

void RepeatStmtNode::dump(std::ostream& os, int indent) const {
    print_indent(os, indent);
    os << "RepeatStmt:\n";
    if (count_expr) count_expr->dump(os, indent + 1);
    print_indent(os, indent);
    os << "Body:\n";
    for (const auto& stmt : body) stmt->dump(os, indent + 1);
}

void CallStmtNode::dump(std::ostream& os, int indent) const {
    print_indent(os, indent);
    switch (kind) {
        case CallKind::ADD:
            os << "CallStmt(add to " << target << "):\n";
            if (expr) expr->dump(os, indent + 1);
            break;
        case CallKind::SUBTRACT:
            os << "CallStmt(subtract from " << target << "):\n";
            if (expr) expr->dump(os, indent + 1);
            break;
        case CallKind::SHOW:
            os << "CallStmt(show):\n";
            if (expr) expr->dump(os, indent + 1);
            break;
        case CallKind::PLAY:
            os << "CallStmt(play " << target << ")\n";
            break;
        case CallKind::SHOW_DISPLAY_AT:
            os << "CallStmt(show display at " << target << ")\n";
            break;
        case CallKind::HIDE_DISPLAY:
            os << "CallStmt(hide display)\n";
            break;
    }
}

void PackageFieldNode::dump(std::ostream& os, int indent) const {
    print_indent(os, indent);
    os << "PackageField(" << key << " = \"" << value << "\")\n";
}

void PackageHeaderNode::dump(std::ostream& os, int indent) const {
    print_indent(os, indent);
    os << "PackageHeader(name=\"" << name << "\", id=\"" << id << "\", label=\"" << label << "\")\n";
}

void ImportNode::dump(std::ostream& os, int indent) const {
    print_indent(os, indent);
    os << "Import(from=\"" << from_module << "\", item=\"" << item << "\")\n";
}

void PermItemNode::dump(std::ostream& os, int indent) const {
    print_indent(os, indent);
    os << "Permission(" << name << " = ";
    if (is_bool) os << (bool_value ? "true" : "false");
    else os << num_value;
    os << ")\n";
}

void PermissionBlockNode::dump(std::ostream& os, int indent) const {
    print_indent(os, indent);
    os << "PermissionBlock:\n";
    for (const auto& it : items) it->dump(os, indent + 1);
}

void HCCSItemNode::dump(std::ostream& os, int indent) const {
    print_indent(os, indent);
    os << "HCCSItem(\"" << hex_code << "\" is \"" << name << "\")\n";
}

void HCCSBlockNode::dump(std::ostream& os, int indent) const {
    print_indent(os, indent);
    os << "HCCSBlock:\n";
    for (const auto& it : items) it->dump(os, indent + 1);
}

void RootDeclNode::dump(std::ostream& os, int indent) const {
    print_indent(os, indent);
    os << "RootDecl(kind=" << kind << ", handle=" << handle << ")\n";
}

void ElementDeclNode::dump(std::ostream& os, int indent) const {
    print_indent(os, indent);
    os << "ElementDecl(kind=" << kind << ", handle=" << handle << ", parent=" << parent_handle << ")\n";
}

void AttrStmtNode::dump(std::ostream& os, int indent) const {
    print_indent(os, indent);
    os << "AttrStmt(" << handle << " " << attr_name << "):\n";
    if (factor_val) factor_val->dump(os, indent + 1);
    else if (expr_val) expr_val->dump(os, indent + 1);
    else if (!keyword_val.empty()) {
        print_indent(os, indent + 1);
        os << "Keyword(" << keyword_val << ")\n";
    }
}

void AttrEndNode::dump(std::ostream& os, int indent) const {
    print_indent(os, indent);
    os << "AttrEnd(" << handle << ")\n";
}

void StateDeclNode::dump(std::ostream& os, int indent) const {
    print_indent(os, indent);
    os << "StateDecl(" << name << "):\n";
    if (expr) expr->dump(os, indent + 1);
}

void HandlerNode::dump(std::ostream& os, int indent) const {
    print_indent(os, indent);
    os << "Handler(" << handle << " is " << event_name << "):\n";
    for (const auto& s : statements) s->dump(os, indent + 1);
}

void ProgramNode::dump(std::ostream& os, int indent) const {
    print_indent(os, indent);
    os << "Program:\n";
    if (header) header->dump(os, indent + 1);
    for (const auto& imp : imports) imp->dump(os, indent + 1);
    if (permissions) permissions->dump(os, indent + 1);
    if (hccs) hccs->dump(os, indent + 1);
    if (root) root->dump(os, indent + 1);
    for (const auto& item : top_items) item->dump(os, indent + 1);
}

} // namespace lore
