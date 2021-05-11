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
    virtual std::string debug() const = 0;
    virtual ~expr() = default;
};
struct assign_expr : expr {
    std::unique_ptr<expr> lhs;
    std::unique_ptr<expr> rhs;

    assign_expr(std::unique_ptr<expr> lhs, std::unique_ptr<expr> rhs) : lhs{std::move(lhs)}, rhs{std::move(rhs)} {}

    std::string debug() const override { return fmt::format("<assign lhs={}, rhs={}>", lhs->debug(), rhs->debug()); }
};

struct binary_expr : expr {
    std::unique_ptr<expr> lhs;
    token op;
    std::unique_ptr<expr> rhs;

    binary_expr(std::unique_ptr<expr> lhs, token t, std::unique_ptr<expr> rhs)
        : lhs{std::move(lhs)}, op{t}, rhs{std::move(rhs)} {}

    std::string debug() const override {
        return fmt::format("<binary lhs={} op={} rhs={}>", lhs->debug(), op, rhs->debug());
    }
};
struct logical_expr : expr {
    std::unique_ptr<expr> lhs;
    token op;
    std::unique_ptr<expr> rhs;

    logical_expr(std::unique_ptr<expr> lhs, token t, std::unique_ptr<expr> rhs)
        : lhs{std::move(lhs)}, op{t}, rhs{std::move(rhs)} {}
    std::string debug() const override {
        return fmt::format("<logical lhs={} op={} rhs={}>", lhs->debug(), op, rhs->debug());
    }
};
struct unary_expr : expr {
    token op;
    std::unique_ptr<expr> operand;

    unary_expr(token t, std::unique_ptr<expr> opr) : op{t}, operand{std::move(opr)} {}

    std::string debug() const override { return fmt::format("<unary op={} operand={}>", op, operand->debug()); }
};
struct bool_expr : expr {
    bool value;
    bool_expr(bool x) : value{x} {}
    bool_expr(token tok) : value{tok == token::true_ ? true : false} {}

    std::string debug() const override { return fmt::format("<bool value={}>", value); }
};
struct return_stmt : expr {
    std::unique_ptr<expr> value;
    return_stmt(std::unique_ptr<expr> v) : value{std::move(v)} {}

    std::string debug() const override { return fmt::format("<return value={}>", value->debug()); }
};
struct num_expr : expr {
    long double value;
    num_expr(const std::string& value) : value{std::stold(value)} {}
    num_expr(long double value) : value{value} {}

    std::string debug() const override { return fmt::format("<number value={}>", value); }
};
struct string_expr : expr {
    std::string value;
    string_expr(const std::string& val) : value{val} {}

    std::string debug() const override { return fmt::format("<string value={}>", value); }
};
struct null_expr : expr {
    std::string debug() const override { return "<null>"; }
};
struct variable_expr : expr {
    std::string name;
    std::unique_ptr<expr> type;
    std::unique_ptr<expr> value;
    bool is_const;
    variable_expr(const std::string& val, std::unique_ptr<expr> t, std::unique_ptr<expr> v, bool i_c)
        : name{val}, type{std::move(t)}, value{std::move(v)}, is_const{i_c} {}

    std::string debug() const override { return fmt::format("<variable name={} value={}>", name, value->debug()); }
};
struct ident_expr : expr {
    std::string name;
    ident_expr(const std::string& val) : name{val} {}
    std::string debug() const override { return fmt::format("<ident value={}>", name); }
};
struct if_stmt : expr {
    std::unique_ptr<expr> condition;
    std::unique_ptr<expr> then_branch;
    std::unique_ptr<expr> else_branch;
    if_stmt(std::unique_ptr<expr> c, std::unique_ptr<expr> t, std::unique_ptr<expr> e)
        : condition{std::move(c)}, then_branch{std::move(t)}, else_branch{std::move(e)} {}

    std::string debug() const override {
        return fmt::format("<if confition={} then={} else={}>", condition->debug(), then_branch->debug(),
                           (else_branch ? else_branch->debug() : "null"));
    }
};
struct call_expr : expr {
    std::unique_ptr<expr> callee;
    std::vector<std::unique_ptr<expr>> arguments;
    call_expr(std::unique_ptr<expr> c, std::vector<std::unique_ptr<expr>> a)
        : callee{std::move(c)}, arguments{std::move(a)} {}

    std::string debug() const override {
        std::string str{};
        if (arguments.size() == 0)
            str = "null";
        else
            for (auto& u : arguments) (str += u->debug()) += ',';
        return fmt::format("<call callee={} arguments{}>", callee->debug(), str);
    }
};
struct argument_expr : expr {
    std::string name;
    std::unique_ptr<expr> type;
    std::unique_ptr<expr> default_;
};
struct function_stmt : expr {
    std::string name;
    std::vector<std::unique_ptr<expr>> arguments;
    std::unique_ptr<expr> body;

    function_stmt(const std::string& n, std::vector<std::unique_ptr<expr>> a, std::unique_ptr<expr> b)
        : name{n}, arguments{std::move(a)}, body{std::move(b)} {}

    std::string debug() const override {
        std::string args{};
        for (auto& arg : arguments) (args += arg->debug()) += ',';
        return fmt::format("<function name={} arguments={} body={}>", name, args.size() > 0 ? args : "null",
                           body->debug());
    }
};
struct for_stmt : expr {
    std::unique_ptr<expr> branch;
    std::unique_ptr<expr> body;

    for_stmt(std::unique_ptr<expr> b, std::unique_ptr<expr> o) : branch{std::move(b)}, body{std::move(o)} {}

    std::string debug() const override {
        return fmt::format("<for condition={} body={}>", branch->debug(), body->debug());
    }
};
struct while_stmt : expr {
    std::unique_ptr<expr> branch;
    std::unique_ptr<expr> body;

    while_stmt(std::unique_ptr<expr> b, std::unique_ptr<expr> o) : branch{std::move(b)}, body{std::move(o)} {}

    std::string debug() const override {
        return fmt::format("<while condition={} body={}>", branch->debug(), body->debug());
    }
};
struct class_expr : expr {
    std::string name;
    std::vector<std::unique_ptr<expr>> methods;
    std::vector<std::unique_ptr<expr>> members;
};
struct access_expr : expr {
    std::unique_ptr<expr> object;
    std::unique_ptr<expr> target;

    std::string debug() const override {
        return fmt::format("<access object={} target={}>", object->debug(), target->debug());
    }
};

struct block_stmt : expr {
    std::vector<std::unique_ptr<expr>> stmts;
    block_stmt(std::vector<std::unique_ptr<expr>> s) : stmts{std::move(s)} {}

    std::string debug() const override {
        std::string out{};
        for (auto& stmt : stmts) {
            (out += stmt->debug()) += ';';
        }
        return out.size() > 0 ? out : "empty";
    }
};
struct range_expr : expr {
    long double min_;
    long double max_;
    std::string debug() const override { return fmt::format("<range min={} max={}>", min_, max_); }
};
struct iterate_expr : expr {
    std::unique_ptr<expr> ident_;
    std::unique_ptr<expr> target;
    std::string debug() const override {
        return fmt::format("<iterate-expr ident={} target={}>", ident_->debug(), target->debug());
    }
};
struct subscript_expr : expr {
    std::unique_ptr<expr> object;
    std::unique_ptr<expr> target;

    std::string debug() const override {
        return fmt::format("<subscript object={} target={}>", object->debug(), target->debug());
    }
};
}  // namespace skai
#endif
