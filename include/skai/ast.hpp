#ifndef SKAI_AST_HPP_739300393
#define SKAI_AST_HPP_739300393
#include <fmt/format.h>

#include <memory>
#include <string>
#include <type_traits>

#include "lexer.hpp"
namespace skai {
enum class ast_t {
    assign_expr,
    binary_expr,
    logical_expr,
    unary_expr,
    bool_expr,
    return_stmt,
    num_expr,
    call_expr,
    null_expr,
    if_stmt,
    while_stmt,
    for_stmt,
    class_expr,
    ident_expr,
    variable_expr,
    string_expr,
    function_stmt,
    access_expr,
    block_stmt,
    iterate_expr,
    range_expr,
    subscript_expr

};
struct expr {
    virtual std::string debug() const = 0;
    virtual ast_t type() const = 0;
    virtual ~expr() = default;
};
struct assign_expr : expr {
    std::shared_ptr<expr> lhs;
    std::shared_ptr<expr> rhs;

    assign_expr(std::shared_ptr<expr> lhs, std::shared_ptr<expr> rhs) : lhs{std::move(lhs)}, rhs{std::move(rhs)} {}

    std::string debug() const override { return fmt::format("<assign lhs={}, rhs={}>", lhs->debug(), rhs->debug()); }
    ast_t type() const override { return ast_t::assign_expr; }
};

struct binary_expr : expr {
    std::shared_ptr<expr> lhs;
    token op;
    std::shared_ptr<expr> rhs;

    binary_expr(std::shared_ptr<expr> lhs, token t, std::shared_ptr<expr> rhs)
        : lhs{std::move(lhs)}, op{t}, rhs{std::move(rhs)} {}

    std::string debug() const override {
        return fmt::format("<binary lhs={} op={} rhs={}>", lhs->debug(), op, rhs->debug());
    }
    ast_t type() const override { return ast_t::binary_expr; }
};
struct logical_expr : expr {
    std::shared_ptr<expr> lhs;
    token op;
    std::shared_ptr<expr> rhs;

    logical_expr(std::shared_ptr<expr> lhs, token t, std::shared_ptr<expr> rhs)
        : lhs{std::move(lhs)}, op{t}, rhs{std::move(rhs)} {}
    std::string debug() const override {
        return fmt::format("<logical lhs={} op={} rhs={}>", lhs->debug(), op, rhs->debug());
    }
    ast_t type() const override { return ast_t::logical_expr; }
};
struct unary_expr : expr {
    token op;
    std::shared_ptr<expr> operand;

    unary_expr(token t, std::shared_ptr<expr> opr) : op{t}, operand{std::move(opr)} {}

    std::string debug() const override { return fmt::format("<unary op={} operand={}>", op, operand->debug()); }
    ast_t type() const override { return ast_t::unary_expr; }
};
struct bool_expr : expr {
    bool value;
    bool_expr(bool x) : value{x} {}
    bool_expr(token tok) : value{tok == token::true_ ? true : false} {}

    std::string debug() const override { return fmt::format("<bool value={}>", value); }
    ast_t type() const override { return ast_t::bool_expr; }
};
struct return_stmt : expr {
    std::shared_ptr<expr> value;
    return_stmt(std::shared_ptr<expr> v) : value{std::move(v)} {}

    std::string debug() const override { return fmt::format("<return value={}>", value->debug()); }
    ast_t type() const override { return ast_t::return_stmt; }
};
struct num_expr : expr {
    int value;
    num_expr(const std::string& value) : value{std::stoi(value)} {}
    num_expr(int value) : value{value} {}

    std::string debug() const override { return fmt::format("<number value={}>", value); }
    ast_t type() const override { return ast_t::num_expr; }
};
struct string_expr : expr {
    std::string value;
    string_expr(const std::string& val) : value{val} {}

    std::string debug() const override { return fmt::format("<string value={}>", value); }
    ast_t type() const override { return ast_t::string_expr; }
};
struct null_expr : expr {
    std::string debug() const override { return "<null>"; }
    ast_t type() const override { return ast_t::null_expr; }
};
struct variable_expr : expr {
    std::string name;
    std::shared_ptr<expr> type_;
    std::shared_ptr<expr> value;
    bool is_const;
    variable_expr(const std::string& val, std::shared_ptr<expr> t, std::shared_ptr<expr> v, bool i_c)
        : name{val}, type_{std::move(t)}, value{std::move(v)}, is_const{i_c} {}

    std::string debug() const override { return fmt::format("<variable name={} value={}>", name, value->debug()); }
    ast_t type() const override { return ast_t::variable_expr; }
};
struct ident_expr : expr {
    std::string name;
    ident_expr(const std::string& val) : name{val} {}
    std::string debug() const override { return fmt::format("<ident value={}>", name); }
    ast_t type() const override { return ast_t::ident_expr; }
};
struct if_stmt : expr {
    std::shared_ptr<expr> condition;
    std::shared_ptr<expr> then_branch;
    std::shared_ptr<expr> else_branch;
    if_stmt(std::shared_ptr<expr> c, std::shared_ptr<expr> t, std::shared_ptr<expr> e)
        : condition{std::move(c)}, then_branch{std::move(t)}, else_branch{std::move(e)} {}

    std::string debug() const override {
        return fmt::format("<if confition={} then={} else={}>", condition->debug(), then_branch->debug(),
                           (else_branch ? else_branch->debug() : "null"));
    }
    ast_t type() const override { return ast_t::if_stmt; }
};
struct call_expr : expr {
    std::shared_ptr<expr> callee;
    std::vector<std::shared_ptr<expr>> arguments;
    call_expr(std::shared_ptr<expr> c, std::vector<std::shared_ptr<expr>> a)
        : callee{std::move(c)}, arguments{std::move(a)} {}

    std::string debug() const override {
        std::string str{};
        if (arguments.size() == 0)
            str = "null";
        else
            for (auto& u : arguments) (str += u->debug()) += ',';
        return fmt::format("<call callee={} arguments={}>", callee->debug(), str);
    }
    ast_t type() const override { return ast_t::call_expr; }
};
struct argument_expr : expr {
    std::string name;
    std::shared_ptr<expr> type;
    std::shared_ptr<expr> default_;
};
struct function_stmt : expr {
    std::string name;
    std::vector<std::shared_ptr<expr>> arguments;
    std::vector<std::shared_ptr<expr>> body;

    function_stmt(const std::string& n, std::vector<std::shared_ptr<expr>> a, std::vector<std::shared_ptr<expr>> b)
        : name{n}, arguments{std::move(a)}, body{std::move(b)} {}

    std::string debug() const override {
        std::string args{};
        std::string bodystr{};
        for (auto& arg : arguments) (args += arg->debug()) += ',';
        for (auto& stmt : body) (bodystr += stmt->debug()) += ',';
        return fmt::format("<function name={} arguments={} body={}>", name, args.size() > 0 ? args : "null", bodystr);
    }
    ast_t type() const override { return ast_t::function_stmt; }
};
struct for_stmt : expr {
    std::shared_ptr<expr> branch;
    std::shared_ptr<expr> body;

    for_stmt(std::shared_ptr<expr> b, std::shared_ptr<expr> o) : branch{std::move(b)}, body{std::move(o)} {}

    std::string debug() const override {
        return fmt::format("<for condition={} body={}>", branch->debug(), body->debug());
    }
    ast_t type() const override { return ast_t::for_stmt; }
};
struct while_stmt : expr {
    std::shared_ptr<expr> branch;
    std::shared_ptr<expr> body;

    while_stmt(std::shared_ptr<expr> b, std::shared_ptr<expr> o) : branch{std::move(b)}, body{std::move(o)} {}

    std::string debug() const override {
        return fmt::format("<while condition={} body={}>", branch->debug(), body->debug());
    }
    ast_t type() const override { return ast_t::while_stmt; }
};
struct class_expr : expr {
    std::string name;
    std::vector<std::shared_ptr<expr>> methods;
    std::vector<std::shared_ptr<expr>> members;
    ast_t type() const override { return ast_t::class_expr; }
};
struct access_expr : expr {
    std::shared_ptr<expr> object;
    std::shared_ptr<expr> target;

    std::string debug() const override {
        return fmt::format("<access object={} target={}>", object->debug(), target->debug());
    }
    ast_t type() const override { return ast_t::access_expr; }
};

struct block_stmt : expr {
    std::vector<std::shared_ptr<expr>> stmts;
    block_stmt(std::vector<std::shared_ptr<expr>> s) : stmts{std::move(s)} {}

    std::string debug() const override {
        std::string out{};
        for (auto& stmt : stmts) {
            (out += stmt->debug()) += ';';
        }
        return out.size() > 0 ? out : "empty";
    }
    ast_t type() const override { return ast_t::block_stmt; }
};
struct range_expr : expr {
    long double min_;
    long double max_;
    std::string debug() const override { return fmt::format("<range min={} max={}>", min_, max_); }
};
struct iterate_expr : expr {
    std::shared_ptr<expr> ident_;
    std::shared_ptr<expr> target;
    std::string debug() const override {
        return fmt::format("<iterate-expr ident={} target={}>", ident_->debug(), target->debug());
    }
};
struct subscript_expr : expr {
    std::shared_ptr<expr> object;
    std::shared_ptr<expr> target;

    std::string debug() const override {
        return fmt::format("<subscript object={} target={}>", object->debug(), target->debug());
    }
};
}  // namespace skai
#endif
