#ifndef SKAI_INTERPRETER_HPP_UEOEPEPE738393
#define SKAI_INTERPRETER_HPP_UEOEPEPE738393
#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "ast.hpp"
#include "error.hpp"
#include "object.hpp"
#include "parser.hpp"
#include "scope.hpp"

namespace skai {
template <class T, class U>
T as(U&& arg) {
    return static_cast<T>(std::forward<U>(arg));
}
namespace builtin {
template <class InterpreterClass>
struct print : object::callable<InterpreterClass> {
    std::size_t arity() override { return 1; }
    std::shared_ptr<object::object> call(InterpreterClass&,
                                         const std::vector<std::shared_ptr<object::object>>& args) override {
        std::cout << args.at(0)->to_string();
        return std::make_shared<object::null>();
    }
    object_t to_underlying() const override { return object_t::function; }
    std::string to_string() const override { return "[pure function]"; }
};

template <class InterpreterClass>
struct println : object::callable<InterpreterClass> {
    std::size_t arity() override { return 1; }
    std::shared_ptr<object::object> call(InterpreterClass&,
                                         const std::vector<std::shared_ptr<object::object>>& args) override {
        std::cout << args.at(0)->to_string() << '\n';
        return std::make_shared<object::null>();
    }
    object_t to_underlying() const override { return object_t::function; }
    std::string to_string() const override { return "[pure function]"; }
};

template <class InterpreterClass>
struct pow : object::callable<InterpreterClass> {
    std::size_t arity() override { return 2; }
    std::shared_ptr<object::object> call(InterpreterClass&,
                                         const std::vector<std::shared_ptr<object::object>>& args) override {
        if (args.at(0).get()->to_underlying() != object_t::integer &&
            args.at(1).get()->to_underlying() != object_t::integer)
            throw skai::exception{"'pow' expected integer arguments type"};
        return std::make_shared<object::integer>(
            std::pow(as<object::integer*>(args.at(0).get())->value, as<object::integer*>(args.at(1).get())->value));
    }

    object_t to_underlying() const override { return object_t::function; }
    std::string to_string() const override { return "[pure function]"; }
};
}  // namespace builtin
struct interpreter {
    interpreter() : m_globals{}, m_ret{std::make_shared<object::null>()} {
        m_globals.define("print", std::make_shared<builtin::print<interpreter>>());
        m_globals.define("println", std::make_shared<builtin::println<interpreter>>());
        m_globals.define("pow", std::make_shared<builtin::pow<interpreter>>());
        m_env = m_globals;
    }

    void interpret(const std::vector<std::shared_ptr<expr>>& exprs) {
        for (auto& elm : exprs) m_eval(std::move(elm));
    }

    std::shared_ptr<object::object> m_eval(std::shared_ptr<expr> expr_) {
        auto expr_o = expr_.get();
        if (expr_o == nullptr) return std::make_shared<object::null>();
        switch (expr_o->type()) {
            default:
                return std::make_shared<object::null>();

            case ast_t::call_expr:
                return m_visit_call(as<call_expr*>(expr_o));

            case ast_t::binary_expr:
                return m_visit_binary_expr(as<binary_expr*>(expr_o));

            case ast_t::logical_expr:
                return m_visit_logical(as<logical_expr*>(expr_o));

            case ast_t::string_expr:
                return std::make_unique<object::string>(as<string_expr*>(expr_o)->value);

            case ast_t::num_expr:
                return std::make_unique<object::integer>(as<num_expr*>(expr_o)->value);

            case ast_t::bool_expr:
                return std::make_unique<object::boolean>(as<bool_expr*>(expr_o)->value);

            case ast_t::null_expr:
                return std::make_unique<object::null>();

            case ast_t::ident_expr:
                return m_visit_ident(as<ident_expr*>(expr_o));

            case ast_t::variable_expr:
                return m_visit_var(as<variable_expr*>(expr_o));

            case ast_t::assign_expr:
                return m_visit_assign(as<assign_expr*>(expr_o));

            case ast_t::function_stmt:
                return m_visit_func(as<function_stmt*>(expr_o));

            case ast_t::if_stmt:
                return m_visit_if_stmt(as<if_stmt*>(expr_o));

            case ast_t::return_stmt:
                return m_visit_return(as<return_stmt*>(expr_o));

            case ast_t::unary_expr:
                return m_visit_unary(as<unary_expr*>(expr_o));

            case ast_t::block_stmt:
                return m_visit_block(as<block_stmt*>(expr_o));

            case ast_t::while_stmt:
                return m_visit_while(as<while_stmt*>(expr_o));
        }
    }

    void m_exec_block(const std::vector<std::shared_ptr<expr>>& exprs, const scope<object::object>& sc) {
        auto prev = m_env;
        m_env = sc;
        for (const auto& elm : exprs) {
            m_eval(elm);
            if (elm->type() == ast_t::return_stmt) break;
        }
        m_env = prev;
    }

    std::shared_ptr<object::object> m_visit_ident(ident_expr* idex) { return m_env.get(idex->name); }

    std::shared_ptr<object::object> m_visit_var(variable_expr* var) {
        std::shared_ptr<object::object> value = nullptr;
        if (var->value != nullptr) value = m_eval(std::move(var->value));
        m_env.define(var->name, std::move(value));
        return std::make_unique<object::null>();
    }

    std::shared_ptr<object::object> m_visit_call(call_expr* cexpr) {
        auto callee = m_eval(std::move(cexpr->callee));
        std::vector<std::shared_ptr<object::object>> args;
        for (auto& arg : cexpr->arguments) args.push_back(m_eval(std::move(arg)));
        auto function = as<object::callable<interpreter>*>(callee.get());
        if (function == nullptr) {
            throw skai::exception{"cannot perfrom call operation on a non-callable object"};
        }
        if (args.size() != function->arity()) {
            throw skai::exception{fmt::format("unmatched arguments count, expected {} arguments got {} instead",
                                              function->arity(), args.size())};
        }
        return function->call(*this, std::move(args));
    }

    std::shared_ptr<object::object> m_visit_assign(assign_expr* aexpr) {
        auto new_value = m_eval(std::move(aexpr->rhs));
        if (aexpr->lhs->type() != ast_t::ident_expr) throw skai::exception{"invalid operand for '='"};
        m_env.assign(as<ident_expr*>(aexpr->lhs.get())->name, new_value);
        return std::make_shared<object::null>();
    }

    std::shared_ptr<object::object> m_visit_if_stmt(if_stmt* stmt) {
        if (m_to_bool(m_eval(stmt->condition))) {
            m_eval(stmt->then_branch);

        } else if (stmt->else_branch != nullptr) {
            m_eval(stmt->else_branch);
        }
        return std::make_shared<object::null>();
    }

    std::shared_ptr<object::object> m_visit_while(while_stmt* stmt) {
        while (m_to_bool(m_eval(stmt->branch))) {
            m_eval(stmt->body);
        }
        return std::make_shared<object::null>();
    }

    std::shared_ptr<object::object> m_visit_block(block_stmt* block) {
        m_exec_block(block->stmts, m_env);
        return std::make_shared<object::null>();
    }

    std::shared_ptr<object::object> m_visit_unary(unary_expr* uexpr) {
        auto target = m_eval(uexpr->operand);
        switch (uexpr->op) {
            case token::minus:
                if (target->to_underlying() == object_t::integer) {
                    return std::make_shared<object::integer>(-as<object::integer*>(target.get())->value);
                }
                throw skai::exception{"invalid operand for token '-'"};
            case token::plus:
                if (target->to_underlying() == object_t::integer) {
                    return std::make_shared<object::integer>(+as<object::integer*>(target.get())->value);
                }
                throw skai::exception{"invalid operand for token '+'"};
            case token::not_:
                if (target->to_underlying() == object_t::boolean) {
                    return std::make_shared<object::boolean>(!as<object::boolean*>(target.get())->value);
                }
                throw skai::exception{"invalid operand for token '!'"};
        }
    }

    std::shared_ptr<object::object> m_visit_return(return_stmt* rtst) {
        m_ret = m_eval(std::move(rtst->value));
        return m_ret;
    }

    std::shared_ptr<object::object> m_visit_func(function_stmt* ftst) {
        auto fnc = std::make_shared<object::function<interpreter>>(ftst, m_env);
        m_env.define(ftst->name, fnc);
        return std::make_shared<object::null>();
    }

    bool m_to_bool(const std::shared_ptr<object::object>& obj) {
        if (obj->to_underlying() == object_t::null) {
            return false;
        } else if (obj->to_underlying() == object_t::boolean) {
            return as<object::boolean*>(obj.get())->value;
        }
        throw skai::exception{"implicit conversions to booleans are disallowed"};
    }

    std::shared_ptr<object::object> m_visit_binary_expr(binary_expr* bin) {
        auto left = m_eval(std::move(bin->lhs));
        auto right = m_eval(std::move(bin->rhs));
        if (left == nullptr && right == nullptr) {
            throw skai::exception{"invalid operands for binary operator"};
        }
        if ((left->to_underlying() != object_t::integer) && (right->to_underlying() != object_t::integer)) {
            throw skai::exception{"invalid operands for binary operator"};
        }
        object::integer left_int = *as<object::integer*>(left.get());
        object::integer right_int = *as<object::integer*>(right.get());
        switch (bin->op) {
            case token::d_eq:
                return std::make_unique<object::boolean>(left_int.value == right_int.value);

            case token::not_eq_:
                return std::make_unique<object::boolean>(left_int.value != right_int.value);

            case token::lt:
                return std::make_unique<object::boolean>(left_int.value < right_int.value);

            case token::gt:
                return std::make_unique<object::boolean>(left_int.value > right_int.value);

            case token::lt_eq:
                return std::make_unique<object::boolean>(left_int.value <= right_int.value);

            case token::gt_eq:
                return std::make_unique<object::boolean>(left_int.value >= right_int.value);

            case token::plus:
                return std::make_unique<object::integer>(left_int.value + right_int.value);

            case token::minus:
                return std::make_unique<object::integer>(left_int.value - right_int.value);

            case token::star:
                return std::make_unique<object::integer>(left_int.value * right_int.value);

            case token::slash:
                return std::make_unique<object::ldouble>(left_int.value / right_int.value);

            case token::mod:
                return std::make_unique<object::integer>(left_int.value % right_int.value);
        }
        return std::make_unique<object::null>();
    }

    std::shared_ptr<object::object> m_visit_logical(logical_expr* expr_) {
        auto left = m_to_bool(m_eval(std::move(expr_->lhs)));
        auto right = m_to_bool(m_eval(std::move(expr_->rhs)));
        switch (expr_->op) {
            case token::and_:
                return std::make_unique<object::boolean>(left && right);
            case token::or_:
                return std::make_unique<object::boolean>(left || right);
        }
        return std::make_unique<object::boolean>(false);
    }

    std::shared_ptr<object::object> get_return() { return m_ret; }

   private:
    scope<object::object> m_globals;
    std::vector<std::shared_ptr<expr>> m_tokens;
    std::shared_ptr<object::object> m_ret;
    scope<object::object> m_env;
};
}  // namespace skai
#endif
