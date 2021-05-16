#ifndef SKAI_INTERPRETER_HPP_UEOEPEPE738393
#define SKAI_INTERPRETER_HPP_UEOEPEPE738393
#include <algorithm>
#include <string>
#include <map>
#include <memory>
#include <skai/libs/builtins.hpp>
#include <skai/utils.hpp>
#include <utility>
#include <vector>

#include "ast.hpp"
#include "error.hpp"
#include "object.hpp"
#include "parser.hpp"
#include "scope.hpp"

namespace skai {
struct interpreter {
    interpreter() : m_globals{}, m_ret{std::make_shared<object::null>()} {
        m_globals.define("print", std::make_shared<builtins::print<interpreter>>());
        m_globals.define("prompt", std::make_shared<builtins::prompt<interpreter>>());
        m_globals.define("random", std::make_shared<builtins::random<interpreter>>());
        m_globals.define("time", std::make_shared<builtins::time<interpreter>>());
        m_globals.define("sleep", std::make_shared<builtins::sleep<interpreter>>());
        m_globals.define("type_of", std::make_shared<builtins::type_of<interpreter>>());
        m_env = m_globals;
    }

    void interpret(const std::vector<std::shared_ptr<expr>>& exprs) {
        for (auto& elm : exprs) m_eval(elm);
    }

    std::shared_ptr<object::object> m_eval(std::shared_ptr<expr> expr_) {
        auto expr_o = expr_.get();
        if (expr_o == nullptr) return std::make_shared<object::null>();

        if (auto fexpr = dynamic_cast<call_expr*>(expr_o))
            return m_visit_call(fexpr);

        else if (auto fexpr = dynamic_cast<binary_expr*>(expr_o))
            return m_visit_binary_expr(fexpr);

        else if (auto fexpr = dynamic_cast<access_expr*>(expr_o))
            return m_visit_access(fexpr);

        else if (auto fexpr = dynamic_cast<logical_expr*>(expr_o))
            return m_visit_logical(fexpr);

        else if (auto fexpr = dynamic_cast<string_expr*>(expr_o))
            return std::make_shared<object::string>(fexpr->value);

        else if (auto fexpr = dynamic_cast<num_expr*>(expr_o))
            return std::make_shared<object::integer>(fexpr->value);

        else if (auto fexpr = dynamic_cast<ldouble_expr*>(expr_o))
            return std::make_shared<object::ldouble>(fexpr->value);

        else if (auto fexpr = dynamic_cast<bool_expr*>(expr_o))
            return std::make_shared<object::boolean>(fexpr->value);

        else if (dynamic_cast<null_expr*>(expr_o))
            return std::make_shared<object::null>();

        else if (auto fexpr = dynamic_cast<variable_expr*>(expr_o))
            return m_visit_var(fexpr);

        else if (auto fexpr = dynamic_cast<ident_expr*>(expr_o))
            return m_visit_ident(fexpr);

        else if (auto fexpr = dynamic_cast<assign_expr*>(expr_o))
            return m_visit_assign(fexpr);

        else if (auto fexpr = dynamic_cast<function_stmt*>(expr_o))
            return m_visit_func(fexpr);

        else if (auto fexpr = dynamic_cast<if_stmt*>(expr_o))
            return m_visit_if_stmt(fexpr);

        else if (auto fexpr = dynamic_cast<return_stmt*>(expr_o))
            return m_visit_return(fexpr);

        else if (auto fexpr = dynamic_cast<for_stmt*>(expr_o))
            return m_visit_for(fexpr);

        else if (auto fexpr = dynamic_cast<while_stmt*>(expr_o))
            return m_visit_while(fexpr);

        else if (auto fexpr = dynamic_cast<block_stmt*>(expr_o))
            return m_visit_block(fexpr);

        else if (auto fexpr = dynamic_cast<unary_expr*>(expr_o))
            return m_visit_unary(fexpr);

        else if (auto fexpr = dynamic_cast<array_expr*>(expr_o))
            return m_visit_arr(fexpr);

        else if (auto fexpr = dynamic_cast<subscript_expr*>(expr_o))
            return m_visit_subsc(fexpr);

        else if (dynamic_cast<break_stmt*>(expr_o))
            is_break = true;

        return std::make_shared<object::null>();
    }

    void m_exec_block(const std::vector<std::shared_ptr<expr>>& exprs, const scope<object::object>& sc, bool is_a_fnc) {
        if (is_a_fnc) {
            auto prev = m_env.get_contents();
            m_env.set_contents(sc.get_contents());
            for (const auto& elm : exprs) {
                if (break_after_ret) break;
                m_eval(elm);
            }
            m_env.set_contents(prev);
        } else {
            for (const auto& elm : exprs) { m_eval(elm); }
        }
    }

    std::shared_ptr<object::object> m_visit_ident(ident_expr* idex) {
        return m_env.get(idex->name);
    }

    std::shared_ptr<object::object> m_visit_arr(array_expr* aexpr) {
        std::vector<std::shared_ptr<object::object>> vals;
        for (auto e : aexpr->elements) vals.push_back(m_eval(e));
        return std::make_shared<object::array>(vals);
    }

    std::shared_ptr<object::object> m_visit_var(variable_expr* var) {
        std::shared_ptr<object::object> value = std::make_shared<object::null>();
        if (var->value != nullptr) value = m_eval(var->value);
        m_env.define(var->name, std::make_shared<object::variable>(var->name, var->is_const, value));
        return std::make_shared<object::null>();
    }

    std::shared_ptr<object::object> m_visit_call(call_expr* cexpr) {
        auto callee = m_eval(cexpr->callee);
        std::vector<std::shared_ptr<object::object>> args;
        for (auto& arg : cexpr->arguments) args.push_back(m_eval(arg));
        if (auto function = dynamic_cast<object::callable<interpreter>*>(callee.get())) {
            if (!function->variadic()) {
                if (args.size() < function->mina() || args.size() > function->maxa()) {
                    std::string pref = args.size() > function->maxa()  ? fmt::format("at most '{}' argument", function->maxa())
                                       : args.size() < function->mina() ? fmt::format("at least '{}' argument", function->mina())
                                                         : fmt::format("'{}' argument", function->maxa());

                    throw skai::exception{
                        fmt::format("unmatched arguments count, expected {}, got {} instead", pref, args.size())};
                }
            }
            return function->call(*this, args);
        }
        throw skai::exception{"cannot perfrom call operation on a non-callable object"};
    }

    std::shared_ptr<object::object> m_visit_assign(assign_expr* aexpr) {
        if (auto ident = dynamic_cast<object::variable*>(m_eval(aexpr->lhs).get())) {
            if (ident->is_const) throw skai::exception{fmt::format("assigning to const variable '{}'", ident->name)};
            auto new_value = m_eval(aexpr->rhs);
            m_env.assign(ident->name, std::make_shared<object::variable>(ident->name, false, new_value));
            return std::make_shared<object::null>();
        }
        throw skai::exception{"invalid operand for '='"};
    }

    std::shared_ptr<object::object> m_visit_access(access_expr* aexpr) {
        auto left = m_eval(aexpr->target);
    }

    std::shared_ptr<object::object> m_visit_subsc(subscript_expr* sexpr) {
        return m_eval(sexpr->object)->operator[](m_eval(sexpr->target));
    }

    std::shared_ptr<object::object> m_visit_if_stmt(if_stmt* stmt) {
        if (stmt->init != nullptr) m_eval(stmt->init);
        if (m_to_bool(m_eval(stmt->condition))) {
            m_eval(stmt->then_branch);

        } else if (stmt->else_branch != nullptr) {
            m_eval(stmt->else_branch);
        }
        return std::make_shared<object::null>();
    }

    std::shared_ptr<object::object> m_visit_while(while_stmt* stmt) {
        within_a_loop = true;
        if (stmt->init != nullptr) m_eval(stmt->init);
        while (m_to_bool(m_eval(stmt->branch)) && !is_break) { m_eval(stmt->body); }
        within_a_loop = false;
        return std::make_shared<object::null>();
    }

    std::shared_ptr<object::object> m_visit_for(for_stmt* stmt) {
        within_a_loop = true;
        for (auto init = m_eval(stmt->init); m_to_bool(m_eval(stmt->condition)) && !is_break;
             init = m_eval(stmt->branch))
            m_eval(stmt->body);

        within_a_loop = false;
        return std::make_shared<object::null>();
    }

    std::shared_ptr<object::object> m_visit_block(block_stmt* block) {
        m_exec_block(block->stmts, m_env, false);
        return std::make_shared<object::null>();
    }

    std::shared_ptr<object::object> m_visit_unary(unary_expr* uexpr) {
        auto target = m_eval(uexpr->operand);
        switch (uexpr->op) {
            case token::minus:
                if (auto i = dynamic_cast<object::integer*>(target.get())) {
                    return std::make_shared<object::integer>(-i->value);
                }
                throw skai::exception{"invalid operand for token '-'"};
            case token::plus:
                if (auto i = dynamic_cast<object::integer*>(target.get())) {
                    return std::make_shared<object::integer>(+i->value);
                }
                throw skai::exception{"invalid operand for token '+'"};
            case token::not_:
                if (auto i = dynamic_cast<object::boolean*>(target.get())) {
                    return std::make_shared<object::boolean>(!i->value);
                }
                throw skai::exception{"invalid operand for token '!'"};
        }
    }

    std::shared_ptr<object::object> m_visit_return(return_stmt* rtst) {
        if (!in_func) throw skai::exception{"return statements are only valid in functions definitions"};
        m_ret = m_eval(rtst->value);
        break_after_ret = true;
        return m_ret;
    }

    std::shared_ptr<object::object> m_visit_func(function_stmt* ftst) {
        auto fnc = std::make_shared<object::function<interpreter>>(*ftst, m_env, ftst->name == "init");
        break_after_ret = false;
        m_env.define(ftst->name, fnc);
        fnc->env = m_env;
        return std::make_shared<object::null>();
    }

    /*std::shared_ptr<object::object> m_visit_class(class_expr* cexpr) {
        auto sc = m_env;
        m_exec_block(cexpr->members, sc, false);
        m_env.define(cexpr->name, std::make_shared<object::class_o<interpreter>>(cexpr->name, sc));
        return std::make_shared<object::null>();
    }
*/
    bool m_to_bool(const std::shared_ptr<object::object>& obj) {
        if (dynamic_cast<object::null*>(obj.get())) {
            return false;
        } else if (auto b = dynamic_cast<object::boolean*>(obj.get())) {
            return b->value;
        }
        throw skai::exception{"implicit conversions to booleans are disallowed"};
    }

    std::shared_ptr<object::object> m_visit_binary_expr(binary_expr* bin) {
        auto left = m_eval(bin->lhs);
        auto right = m_eval(bin->rhs);
#define OP_(tok, op) \
    case token::tok: \
        return left.get()->operator op(right);

#define OP(tok, op) \
    OP_(tok, op)    \
    OP_(tok##_eq, op## =)

        switch (bin->op) {
            OP(plus, +)
            OP(minus, -)
            OP(star, *)
            OP(slash, /)
            OP(b_or, |)
            OP(b_and, &)
            OP(mod, %)
            OP_(d_eq, ==)
            OP_(not_eq_, !=)
            OP(gt, >)
            OP(lt, <)
        }
    }

    std::shared_ptr<object::object> m_visit_logical(logical_expr* expr_) {
        auto left = m_to_bool(m_eval(expr_->lhs));
        auto right = m_to_bool(m_eval(expr_->rhs));
        switch (expr_->op) {
            case token::and_:
                return std::make_shared<object::boolean>(left && right);
            case token::or_:
                return std::make_shared<object::boolean>(left || right);
        }
        return std::make_shared<object::boolean>(false);
    }

    std::shared_ptr<object::object> get_return() {
        return m_ret;
    }
    void set_ret(std::shared_ptr<object::object> obj) {
        m_ret = obj;
    }
    scope<object::object> get_env() {
        return m_env;
    }

    void set_in_func(bool x) {
        in_func = x;
    }
    bool get_in_func() const {
        return in_func;
    }

   private:
    scope<object::object> m_globals;
    std::vector<std::shared_ptr<expr>> m_tokens;
    std::shared_ptr<object::object> m_ret;
    scope<object::object> m_env;
    bool is_break{};
    bool within_a_loop{};
    bool break_after_ret{};
    bool in_func{};
};
}  // namespace skai
#endif
