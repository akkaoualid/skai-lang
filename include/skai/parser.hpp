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
    parser(const std::vector<token_handler> tkns, const std::string& file) : m_src{tkns}, m_file{file} {}

   private:
    auto m_get() { return m_src.at(at_end() ? m_src.size() - 1 : m_pos); }
    void m_advance(std::size_t x = 1) {
        if (!at_end())
            m_pos += x;
        else
            return;
    }
    [[noreturn]] void m_error(const std::string& msg) {
        throw skai::exception{
            fmt::format("{}:{}:{} - {}", m_get().loc.file, m_get().loc.line, m_get().loc.column, msg)};
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
            return m_previous();
        }
        m_error(msg);
    }

    auto expression() { return assignment(); }

    std::shared_ptr<expr> equality() {
        std::shared_ptr<expr> expr_ = comparison();
        while (m_match(token::not_eq_, token::d_eq)) {
            auto oper = m_previous();
            std::shared_ptr<expr> right = comparison();
            expr_ = std::make_shared<binary_expr>(std::move(expr_), oper.token, std::move(right));
        }
        return expr_;
    }

    std::shared_ptr<expr> declaration() {
        if (m_match(token::let)) {
            return var_declaration();
        } else if (m_match(token::fun)) {
            return function_stmt_();
        }
        return statement();
    }

    std::shared_ptr<expr> statement() {
        if (m_match(token::if_)) {
            return if_stmt_();
        } else if (m_match(token::lbracket)) {
            return std::make_shared<block_stmt>(block_stmt{block()});
        } else if (m_match(token::while_)) {
            return while_stmt_();
        } else if (m_match(token::return_)) {
            return return_stmt_();
        }
        return expr_stmt();
    }

    std::shared_ptr<expr> parse_arg() {}

    std::shared_ptr<expr> return_stmt_() {
        std::shared_ptr<expr> value = nullptr;
        if (!m_get().is(token::scolon)) value = expression();
        consume(token::scolon, "expected ';' after return  expression");
        return std::make_shared<return_stmt>(return_stmt{std::move(value)});
    }

    std::shared_ptr<expr> while_stmt_() {
        auto expr_ = expression();
        return std::make_shared<while_stmt>(std::move(expr_), statement());
    }

    std::shared_ptr<expr> expr_stmt() {
        auto expr_ = expression();
        consume(token::scolon, "expected ';' after epxression");
        return expr_;
    }

    std::shared_ptr<expr> assignment() {
        auto expr_ = or_expr();
        if (m_match(token::eq)) {
            auto val = assignment();
            expr_ = std::make_shared<assign_expr>(std::move(expr_), std::move(val));
        }
        return expr_;
    }

    std::shared_ptr<expr> or_expr() {
        auto expr_ = and_expr();
        while (m_match(token::or_)) {
            auto oper = m_previous();
            auto right = and_expr();
            expr_ = std::make_shared<logical_expr>(std::move(expr_), oper.token, std::move(right));
        }
        return expr_;
    }
    std::shared_ptr<expr> and_expr() {
        auto expr_ = equality();
        while (m_match(token::and_)) {
            auto oper = m_previous();
            auto right = equality();
            expr_ = std::make_shared<logical_expr>(std::move(expr_), oper.token, std::move(right));
        }
        return expr_;
    }
    std::shared_ptr<expr> var_declaration() {
        // TODO: add types and const support
        auto name = consume(token::identifier, "expected identifier for variable name");
        std::shared_ptr<expr> init = nullptr;
        std::shared_ptr<expr> type = nullptr;
        bool is_const = false;
        if (m_match(token::eq)) {
            init = expression();
        }
        consume(token::scolon, "expected ';' after variable declaration");
        return std::make_shared<variable_expr>(name.str, std::move(type), std::move(init), is_const);
    }

    std::shared_ptr<expr> if_stmt_() {
        std::shared_ptr<expr> cond = expression();
        std::shared_ptr<expr> then = statement();
        std::shared_ptr<expr> else_ = nullptr;
        if (m_match(token::else_)) {
            else_ = statement();
        }
        return std::make_shared<if_stmt>(std::move(cond), std::move(then), std::move(else_));
    }

    std::shared_ptr<expr> function_stmt_() {
        auto name = consume(token::identifier, "expected identifier");
        std::vector<std::shared_ptr<expr>> params;
        consume(token::lparen, "expected '(' after function");
        if (!m_get().is(token::rparen)) {
            do {
                if (params.size() > 255) throw skai::exception{"can't have more than 255 parameters"};
                consume(token::identifier, "expected identifier");
                params.push_back(std::make_shared<ident_expr>(m_previous().str));
            } while (m_match(token::comma));
        }
        consume(token::rparen, "expected ')' after argument list");
        consume(token::lbracket, "expected '{' after argument list");
        return std::make_shared<function_stmt>(name.str, std::move(params), block());
    }
    std::vector<std::shared_ptr<expr>> block() {
        std::vector<std::shared_ptr<expr>> stmts;
        while (m_get().isnot(token::rbracket) && !at_end()) {
            stmts.emplace_back(declaration());
        }
        consume(token::rbracket, "expectd '}' after block statement");
        return stmts;
    }

    std::shared_ptr<expr> comparison() {
        auto expr_ = term();
        while (m_match(token::lt, token::lt_eq, token::gt, token::gt_eq)) {
            auto oper = m_previous();
            auto right = term();
            expr_ = std::make_shared<binary_expr>(std::move(expr_), oper.token, std::move(right));
        }
        return expr_;
    }
    std::shared_ptr<expr> term() {
        auto expr_ = factor();
        while (m_match(token::plus, token::minus)) {
            auto oper = m_previous();
            auto right = factor();
            expr_ = std::make_shared<binary_expr>(std::move(expr_), oper.token, std::move(right));
        }
        return expr_;
    }
    std::shared_ptr<expr> factor() {
        auto expr_ = unary();
        while (m_match(token::slash, token::star, token::mod)) {
            auto oper = m_previous();
            auto right = unary();
            expr_ = std::make_shared<binary_expr>(std::move(expr_), oper.token, std::move(right));
        }
        return expr_;
    }
    std::shared_ptr<expr> unary() {
        if (m_match(token::not_, token::minus, token::plus)) {
            auto oper = m_previous();
            return std::make_shared<unary_expr>(oper.token, unary());
        }
        return call();
    }

    std::shared_ptr<expr> call() {
        auto expr_ = primary();
        std::vector<std::shared_ptr<expr>> args;
        while (true) {
            if (m_match(token::lparen)) {
                if (!m_get().is(token::rparen)) {
                    do {
                        if (args.size() > 255) m_error("can't have more than 255 arguments");
                        args.push_back(expression());
                    } while (m_match(token::comma));
                }
                consume(token::rparen, "expected ')' after argument list");
                expr_ = std::make_shared<call_expr>(std::move(expr_), std::move(args));
            } else {
                break;
            }
        }
        return expr_;
    }

    std::shared_ptr<expr> primary() {
        if (m_match(token::true_, token::false_)) {
            return std::make_shared<bool_expr>(m_previous().token);
        }
        if (m_match(token::null)) {
            return std::make_shared<null_expr>();
        }
        if (m_match(token::number)) {
            return std::make_shared<num_expr>(m_previous().str);
        }
        if (m_match(token::string)) {
            return std::make_shared<string_expr>(m_previous().str);
        }
        if (m_match(token::lparen)) {
            auto expr_ = expression();
            if (!m_match(token::rparen)) {
                m_error("expected ')' after expression");
            }
            return expr_;
        }
        if (m_match(token::identifier)) {
            return std::make_shared<ident_expr>(m_previous().str);
        }
        m_error(fmt::format("unexpected token {}", m_get().str));
    }

    std::vector<token_handler> m_src;
    std::string m_file;
    std::size_t m_pos = 0;
    std::size_t line = 1;

   public:
    auto parse() {
        std::vector<std::shared_ptr<expr>> stmts;
        while (!at_end()) stmts.emplace_back(declaration());
        return stmts;
    }
};
}  // namespace skai
#endif
