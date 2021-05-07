#ifndef SKAI_PARSER_HPP_739300303
#define SKAI_PARSER_HPP_739300303
#include <memory>
#include <string>
#include <vector>

#include "ast.hpp"
#include "error.hpp"
#include "lexer.hpp"
#include "sloc.hpp"
namespace skai {
struct parser {
    parser(const std::vector<token_handler> tkns, const std::string& file) : m_src{tkns} {}

   private:
    auto m_get() { return m_src.at(at_end() ? m_src.size() - 1 : m_pos); }
    void m_advance(std::size_t x = 1) {
        if (!at_end())
            m_pos += x;
        else
            return;
    }
    auto m_peek(std::size_t x = 1) { return m_src.at(m_pos + x); }
    auto m_previous(std::size_t x = 1) { return m_src.at(m_src.size() == 0 ? 0 : m_pos - x); }
    bool at_end() { return m_pos >= m_src.size(); }
    template <class... T>
    bool m_match(T... toks) {
        if (m_get().is(toks...) && !at_end()) {
            m_advance();
            return true;
        }
        return false;
    }

    auto consume(token tok, const std::string& msg) {
        if (m_match(tok)) {
            return m_get();
        }
        throw skai::exception{msg};
    }

    auto expression() { return equality(); }

    std::unique_ptr<expr> equality() {
        std::unique_ptr<expr> expr_ = comparison();
        while (m_match(token::not_eq_, token::d_eq)) {
            auto oper = m_previous();
            std::unique_ptr<expr> right = comparison();
            expr_ = std::make_unique<binary_expr>(std::move(expr_), oper.token, std::move(right));
        }
        return expr_;
    };

    std::unique_ptr<expr> declaration() {
        if (m_match(token::dec)) {
            return var_declaration();
        }
        return statement();
    }

    std::unique_ptr<expr> statement() {
        if (m_match(token::if_)) {
            return if_stmt_();
        }
    }

    std::unique_ptr<expr> var_declaration() {
        // TODO: add types and const support
        auto name = consume(token::identifier, "expected identifier for variable name");
        std::unique_ptr<expr> init = nullptr;
        std::unique_ptr<expr> type = nullptr;
        bool is_const = false;
        if (m_get().is(token::eq)) {
            init = expression();
        }
        consume(token::scolon, "expected ';' after variable declaration");
        return std::make_unique<variable_expr>(name.str, std::move(type), std::move(init), is_const);
    }

    std::unique_ptr<expr> if_stmt_() {
        consume(token::lparen, "expected '(' before if condition");
        std::unique_ptr<expr> cond = expression();
        consume(token::rparen, "expected ')' after if statement body");
        std::unique_ptr<expr> then = statement();
        std::unique_ptr<expr> else_ = nullptr;
        if (m_match(token::else_)) {
            else_ = statement();
        }
        return std::make_unique<if_stmt>(std::move(cond), std::move(then), std::move(else_));
    }

    std::vector<std::unique_ptr<expr>> block() {
        std::vector<std::unique_ptr<expr>> stmts;
        while (m_get().isnot(token::rbracket) && !at_end()) {
            stmts.emplace_back(declaration());
        }
        consume(token::rbracket, "expectd '}' after block statement");
        return stmts;
    }

    std::unique_ptr<expr> comparison() {
        auto expr_ = term();
        while (m_match(token::lt, token::lt_eq, token::gt, token::gt_eq)) {
            auto oper = m_previous();
            auto right = term();
            expr_ = std::make_unique<binary_expr>(std::move(expr_), oper.token, std::move(right));
        }
        return expr_;
    }
    std::unique_ptr<expr> term() {
        auto expr_ = factor();
        while (m_match(token::plus, token::minus)) {
            auto oper = m_previous();
            auto right = factor();
            expr_ = std::make_unique<binary_expr>(std::move(expr_), oper.token, std::move(right));
        }
        return expr_;
    }
    std::unique_ptr<expr> factor() {
        auto expr_ = unary();
        while (m_match(token::slash, token::star)) {
            auto oper = m_previous();
            auto right = unary();
            expr_ = std::make_unique<binary_expr>(std::move(expr_), oper.token, std::move(right));
        }
        return expr_;
    }
    std::unique_ptr<expr> unary() {
        if (m_get().is(token::not_, token::minus, token::plus)) {
            auto oper = m_previous();
            return std::make_unique<unary_expr>(oper.token, std::move(unary()));
        }
        return primary();
    }
    std::unique_ptr<expr> primary() {
        if (m_match(token::true_, token::false_)) {
            return std::make_unique<bool_expr>(m_get().token);
        } else if (m_match(token::null)) {
            return std::make_unique<null_expr>();
        } else if (m_match(token::number)) {
            return std::make_unique<num_expr>(m_previous().str);
        } else if (m_match(token::string)) {
            return std::make_unique<string_expr>(m_previous().str);
        } else if (m_match(token::lparen)) {
            auto expr_ = expression();
            if (!m_match(token::rparen)) {
                throw skai::exception{fmt::format("{}:{}: expexted ')' after expression", m_file, m_pos)};
            }
            return expr_;
        } else if (m_match(token::identifier)) {
            return std::make_unique<ident_expr>(m_get().str);
        }
        throw skai::exception{"expected expression"};
    }

    std::vector<token_handler> m_src;
    std::string m_file;
    std::size_t m_pos = 0;

   public:
    auto parse() {
        std::vector<std::unique_ptr<expr>> stmts;
        while (!at_end()) stmts.emplace_back(declaration());
        return stmts;
    }
};
}  // namespace skai
#endif
