#ifndef SKAI_AST_HPP_739300393
#define SKAI_AST_HPP_739300393
#include <fmt/format.h>

#include <memory>
#include <string>
#include <type_traits>

#include "lexer.hpp"
namespace skai {
template <class T, class U>
bool instance_of(U&& d) {
    return !std::is_same_v<decltype(static_cast<T*>(d)), void*>;
}
struct expr {
    virtual ~expr() = default;
};
struct assign_expr : expr {
    std::unique_ptr<expr> lhs;
    std::unique_ptr<expr> rhs;

    assign_expr(std::unique_ptr<expr> lhs, std::unique_ptr<expr> rhs) : lhs{std::move(lhs)}, rhs{std::move(rhs)} {}
};

struct binary_expr : expr {
    std::unique_ptr<expr> lhs;
    token op;
    std::unique_ptr<expr> rhs;

    binary_expr(std::unique_ptr<expr> lhs, token t, std::unique_ptr<expr> rhs)
        : lhs{std::move(lhs)}, op{t}, rhs{std::move(rhs)} {}
};
struct unary_expr : expr {
    token op;
    std::unique_ptr<expr> operand;

    unary_expr(token t, std::unique_ptr<expr> opr) : op{t}, operand{std::move(opr)} {}
};
struct bool_expr : expr {
    bool value;
    bool_expr(bool x) : value{x} {}
    bool_expr(token tok) : value{tok == token::true_ ? true : false} {}
};
struct num_expr : expr {
    long double value;
    num_expr(const std::string& value) : value{std::stold(value)} {}
    num_expr(long double value) : value{value} {}
};
struct string_expr : expr {
    std::string value;
    string_expr(const std::string& val) : value{val} {}
};
struct null_expr : expr {};
struct variable_expr : expr {
    std::string name;
    std::unique_ptr<expr> type;
    std::unique_ptr<expr> value;
    bool is_const;
    variable_expr(const std::string& val, std::unique_ptr<expr> t, std::unique_ptr<expr> v, bool i_c)
        : name{val}, type{std::move(t)}, value{std::move(v)}, is_const{i_c} {}
};
struct ident_expr : expr {
    std::string name;
    ident_expr(const std::string& val) : name{val} {}
};
struct if_stmt : expr {
    std::unique_ptr<expr> condition;
    std::unique_ptr<expr> then_branch;
    std::unique_ptr<expr> else_branch;
    if_stmt(std::unique_ptr<expr> c, std::unique_ptr<expr> t, std::unique_ptr<expr> e)
        : condition{std::move(c)}, then_branch{std::move(t)}, else_branch{std::move(e)} {}
};
struct call_expr : expr {
    std::string calle;
    std::vector<std::unique_ptr<expr>> arguments;
};
struct argument_expr : expr {
    std::string name;
    std::unique_ptr<expr> type;
    std::unique_ptr<expr> default_;
};
struct function_expr : expr {
    std::string name;
    std::vector<std::unique_ptr<expr>> body;
    std::vector<std::unique_ptr<expr>> arguments;
    std::unique_ptr<expr> return_stmt;
};
struct for_stmt : expr {
    std::unique_ptr<expr> branch;
    std::vector<std::unique_ptr<expr>> body;
};
struct while_stmt : expr {
    std::unique_ptr<expr> branch;
    std::vector<std::unique_ptr<expr>> body;
};
struct class_expr : expr {
    std::string name;
    std::vector<std::unique_ptr<expr>> methods;
    std::vector<std::unique_ptr<expr>> members;
};
struct access_expr : expr {
    std::unique_ptr<expr> object;
    std::unique_ptr<expr> target;
};
}  // namespace skai
#endif
