#include "checker.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace lore {

static bool file_exists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}

Checker::Checker(const std::string& base_dir) : base_dir_(base_dir) {}

void Checker::check_program(const std::shared_ptr<ProgramNode>& program) {
    if (!program) return;

    if (program->permissions) {
        check_permissions(program->permissions);
    }
    if (program->hccs) {
        check_hccs(program->hccs);
    }
    if (program->root) {
        check_root(program->root);
    }
    check_top_items(program->top_items);
}

void Checker::check_permissions(const std::shared_ptr<PermissionBlockNode>& permissions) {
    bool has_show = false;
    bool has_output = false;

    for (const auto& item : permissions->items) {
        if (item->name == "ShowDisplay" && item->is_bool && item->bool_value) {
            has_show = true;
        } else if (item->name == "OutputDisplay" && item->is_bool && item->bool_value) {
            has_output = true;
        } else if (item->name != "InputHarness" && item->name != "eachtimevalue" &&
                   item->name != "ShowDisplay" && item->name != "OutputDisplay") {
            warnings_.push_back(item->pos.to_string() + ": Unknown permission '" + item->name + "'.");
        }
    }

    if (has_show && has_output) {
        warnings_.push_back(permissions->pos.to_string() +
            ": Warning: 'ShowDisplay : true' already includes 'OutputDisplay : true'; writing both is unnecessary.");
    }
}

void Checker::check_hccs(const std::shared_ptr<HCCSBlockNode>& hccs) {
    for (const auto& item : hccs->items) {
        if (item->hex_code.empty() || item->hex_code[0] != '#' || item->hex_code.length() != 7) {
            throw CompileError(item->pos, "Invalid hex colour format '" + item->hex_code + "'. Expected '#rrggbb'.");
        }
        hccs_colors_.insert(item->name);
    }
}

void Checker::check_root(const std::shared_ptr<RootDeclNode>& root) {
    element_kinds_[root->handle] = root->kind;
    current_attr_handle_ = root->handle;
}

void Checker::check_top_items(const std::vector<std::shared_ptr<ASTNode>>& items) {
    for (const auto& item : items) {
        if (auto el = std::dynamic_pointer_cast<ElementDeclNode>(item)) {
            check_element_decl(el);
        } else if (auto attr = std::dynamic_pointer_cast<AttrStmtNode>(item)) {
            check_attr_stmt(attr);
        } else if (auto end = std::dynamic_pointer_cast<AttrEndNode>(item)) {
            check_attr_end(end);
        } else if (auto st = std::dynamic_pointer_cast<StateDeclNode>(item)) {
            check_state_decl(st);
        } else if (auto h = std::dynamic_pointer_cast<HandlerNode>(item)) {
            check_handler(h);
        } else if (auto call = std::dynamic_pointer_cast<CallStmtNode>(item)) {
            check_call_stmt(call);
        }
    }
}

void Checker::check_element_decl(const std::shared_ptr<ElementDeclNode>& el) {
    if (element_kinds_.find(el->parent_handle) == element_kinds_.end()) {
        throw CompileError(el->pos, "Parent element '" + el->parent_handle + "' does not exist. Cannot put '" + el->handle + "' inside it.");
    }
    if (element_kinds_.find(el->handle) != element_kinds_.end()) {
        throw CompileError(el->pos, "Duplicate element handle '" + el->handle + "'.");
    }

    element_kinds_[el->handle] = el->kind;
    current_attr_handle_ = el->handle;
}

void Checker::check_attr_stmt(const std::shared_ptr<AttrStmtNode>& attr) {
    if (element_kinds_.find(attr->handle) == element_kinds_.end()) {
        throw CompileError(attr->pos, "Element handle '" + attr->handle + "' has not been declared.");
    }

    // Check images
    if (attr->attr_name == "image" || attr->attr_name == "image:pressed" || attr->attr_name == "image:disabled") {
        if (attr->expr_val && attr->expr_val->first && attr->expr_val->first->first) {
            auto factor = attr->expr_val->first->first;
            if (factor->kind == FactorKind::STRING) {
                check_image_path(attr->pos, factor->text);
            }
        }
    }

    // Check color specifications
    if (attr->attr_name == "colour" || attr->attr_name == "color" || attr->attr_name == "image-tint") {
        if (attr->expr_val && attr->expr_val->first && attr->expr_val->first->first) {
            auto factor = attr->expr_val->first->first;
            if (factor->kind == FactorKind::COLOR_REF) {
                std::string cname = factor->text.substr(3); // after CS:
                if (hccs_colors_.find(cname) == hccs_colors_.end()) {
                    throw CompileError(attr->pos, "Unknown colour reference '" + factor->text + "'. It must be defined in the HCCS block.");
                }
            }
        }
    }

    if (attr->expr_val) {
        check_expr(attr->expr_val);
    }
}

void Checker::check_attr_end(const std::shared_ptr<AttrEndNode>& end) {
    if (!current_attr_handle_.empty() && end->handle != current_attr_handle_) {
        throw CompileError(end->pos, "Mismatched attribute terminator: expected '.end" + current_attr_handle_ + "att.' but found '.end" + end->handle + "att.'.");
    }
    if (element_kinds_.find(end->handle) == element_kinds_.end()) {
        throw CompileError(end->pos, "Unknown element handle '" + end->handle + "' in attribute terminator.");
    }
    current_attr_handle_.clear();
}

void Checker::check_image_path(const SourcePos& pos, const std::string& path) {
    // 1. Allowed extensions
    std::string lower = path;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    bool valid_ext = false;
    for (const auto& ext : {".png", ".jpg", ".jpeg", ".webp"}) {
        if (lower.length() >= std::string(ext).length() &&
            lower.compare(lower.length() - std::string(ext).length(), std::string(ext).length(), ext) == 0) {
            valid_ext = true;
            break;
        }
    }

    if (!valid_ext) {
        throw CompileError(pos, "Invalid image extension for '" + path + "'. Allowed extensions: .png, .jpg, .jpeg, .webp.");
    }

    // 2. Existence check
    std::vector<std::string> candidates = {
        path,
        base_dir_ + "/" + path,
        "../" + path,
        "examples/" + path,
        "../examples/" + path,
        "spec/" + path,
        "../spec/" + path
    };

    bool found = false;
    for (const auto& cand : candidates) {
        if (file_exists(cand)) {
            found = true;
            break;
        }
    }

    if (!found) {
        throw CompileError(pos, "Image file not found: '" + path + "'.");
    }
}

void Checker::check_state_decl(const std::shared_ptr<StateDeclNode>& state) {
    if (states_.find(state->name) != states_.end()) {
        throw CompileError(state->pos, "Duplicate state variable '" + state->name + "'.");
    }
    StateType t = check_expr(state->expr);
    states_[state->name] = t;
}

void Checker::check_handler(const std::shared_ptr<HandlerNode>& handler) {
    if (element_kinds_.find(handler->handle) == element_kinds_.end()) {
        throw CompileError(handler->pos, "Element handle '" + handler->handle + "' in event handler has not been declared.");
    }

    const std::string& ev = handler->event_name;
    if (ev != "tapped" && ev != "pressed" && ev != "released" && ev != "changed" && ev != "input") {
        throw CompileError(handler->pos, "Unknown event name '" + ev + "'. Expected 'tapped', 'pressed', 'released', 'changed', or 'input'.");
    }

    for (const auto& stmt : handler->statements) {
        check_statement(stmt);
    }
}

void Checker::check_statement(const std::shared_ptr<StatementNode>& stmt) {
    if (auto assign = std::dynamic_pointer_cast<AssignStmtNode>(stmt)) {
        check_assign_stmt(assign);
    } else if (auto if_st = std::dynamic_pointer_cast<IfStmtNode>(stmt)) {
        check_if_stmt(if_st);
    } else if (auto rep = std::dynamic_pointer_cast<RepeatStmtNode>(stmt)) {
        check_repeat_stmt(rep);
    } else if (auto call = std::dynamic_pointer_cast<CallStmtNode>(stmt)) {
        check_call_stmt(call);
    }
}

void Checker::check_assign_stmt(const std::shared_ptr<AssignStmtNode>& assign) {
    auto it = states_.find(assign->target);
    if (it == states_.end()) {
        throw CompileError(assign->pos, "Cannot assign to undeclared state variable '" + assign->target + "'.");
    }
    check_expr(assign->expr);
}

void Checker::check_if_stmt(const std::shared_ptr<IfStmtNode>& if_stmt) {
    if (if_stmt->condition) {
        check_condition(if_stmt->condition);
    }
    for (const auto& s : if_stmt->then_branch) {
        check_statement(s);
    }
    for (const auto& s : if_stmt->otherwise_branch) {
        check_statement(s);
    }
}

void Checker::check_repeat_stmt(const std::shared_ptr<RepeatStmtNode>& repeat) {
    if (repeat->count_expr) {
        check_expr(repeat->count_expr);
    }
    for (const auto& s : repeat->body) {
        check_statement(s);
    }
}

void Checker::check_call_stmt(const std::shared_ptr<CallStmtNode>& call) {
    if (call->kind == CallKind::ADD || call->kind == CallKind::SUBTRACT) {
        if (states_.find(call->target) == states_.end()) {
            throw CompileError(call->pos, "Target state variable '" + call->target + "' does not exist.");
        }
        check_expr(call->expr);
    } else if (call->kind == CallKind::SHOW && call->expr) {
        check_expr(call->expr);
    } else if (call->kind == CallKind::SHOW_DISPLAY_AT) {
        if (call->target != "input" && call->target != "process" && call->target != "output") {
            throw CompileError(call->pos, "Invalid stage '" + call->target + "'. Expected 'input', 'process', or 'output'.");
        }
    }
}

void Checker::check_condition(const std::shared_ptr<ConditionNode>& cond) {
    if (cond->left) check_expr(cond->left);
    if (cond->right) check_expr(cond->right);
    if (cond->next) check_condition(cond->next);
}

StateType Checker::check_expr(const std::shared_ptr<ExprNode>& expr) {
    if (!expr) return StateType::UNKNOWN;
    StateType t = check_term(expr->first);
    for (const auto& item : expr->rest) {
        check_term(item.term);
    }
    return t;
}

StateType Checker::check_term(const std::shared_ptr<TermNode>& term) {
    if (!term) return StateType::UNKNOWN;
    StateType t = check_factor(term->first);
    for (const auto& item : term->rest) {
        check_factor(item.factor);
    }
    return t;
}

StateType Checker::check_factor(const std::shared_ptr<FactorNode>& factor) {
    if (!factor) return StateType::UNKNOWN;
    switch (factor->kind) {
        case FactorKind::NUMBER:
            return StateType::INT;
        case FactorKind::DIMENSION:
            return StateType::INT;
        case FactorKind::STRING:
            return StateType::STRING;
        case FactorKind::COLOR_REF: {
            std::string name = factor->text.substr(3);
            if (hccs_colors_.find(name) == hccs_colors_.end()) {
                throw CompileError(factor->pos, "Unknown colour reference '" + factor->text + "'. It must be defined in the HCCS block.");
            }
            return StateType::STRING;
        }
        case FactorKind::IDENT: {
            if (states_.find(factor->text) == states_.end()) {
                throw CompileError(factor->pos, "Unknown variable or state '" + factor->text + "'.");
            }
            return states_[factor->text];
        }
        case FactorKind::GROUPED:
            return check_expr(factor->grouped_expr);
        default:
            return StateType::UNKNOWN;
    }
}

} // namespace lore
