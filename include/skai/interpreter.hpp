#ifndef SKAI_INTERPRETER_HPP_UEOEPEPE738393
#define SKAI_INTERPRETER_HPP_UEOEPEPE738393
#include <map>
#include <memory>
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
struct interpreter {
    void visit_function(function_stmt* fst) {
        m_env.define(fst->name, std::make_unique<object::function<interpreter>>(fst, m_env));
    }

    void m_exec(std::unique_ptr<expr> stmt_) {
        auto stmt = stmt_.get();
        if (instance_of<if_stmt>(stmt)) {
            m_visit_if_stmt(as<if_stmt*>(stmt));
        }
    }

    std::unique_ptr<object::object> m_eval(std::unique_ptr<expr> expr_) {}

    void m_visit_if_stmt(if_stmt* if_stmt_) {
        if (m_to_bool(m_eval(std::move(if_stmt_->condition)))) {
            m_exec(std::move(if_stmt_->then_branch));
        } else if (if_stmt_->else_branch != nullptr) {
            m_exec(std::move(if_stmt_->else_branch));
        }
    }

    bool m_to_bool(std::unique_ptr<object::object> obj) {
        if (instance_of<object::null*>(obj.get())) {
            return false;
        } else if (instance_of<object::boolean*>(obj.get())) {
            return as<object::boolean*>(obj.get())->value;
        }
        throw skai::exception{"implicit conversions to booleans are disallowed"};
    }

    std::unique_ptr<object::object> m_visit_logical(logical_expr* expr_) {
        auto left = m_to_bool(m_eval(std::move(expr_->lhs)));
        auto right = m_to_bool(m_eval(std::move(expr_->rhs)));
        switch (expr_->op) {
            case token::and_:
                return std::make_unique<object::boolean>(left && right);
            case token::or_:
                return std::make_unique<object::boolean>(left || right);
        }
    }

    std::unique_ptr<object::object> get_return() { return std::move(m_ret); }

   private:
    scope<object::object> m_globals;
    std::vector<std::unique_ptr<expr>> m_tokens;
    std::unique_ptr<object::object> m_ret;
    scope<object::object> m_env = m_globals;
};
}  // namespace skai
#endif
