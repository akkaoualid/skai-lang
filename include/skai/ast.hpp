#ifndef SKAI_AST_HPP_739300393
#define SKAI_AST_HPP_739300393
#include <fmt/format.h>

#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>

#include "lexer.hpp"
namespace skai {
struct expr {
    virtual std::string debug() const = 0;
    virtual ~expr() = default;
};
struct assign_expr : expr {
    std::shared_ptr<expr> lhs;
    std::shared_ptr<expr> rhs;

    assign_expr(std::shared_ptr<expr> lhs, std::shared_ptr<expr> rhs) : lhs{(lhs)}, rhs{(rhs)} {}

    std::string debug() const override {
        return fmt::format("assgin(left={}, right{})", lhs->debug(), rhs->debug());
    }
};

struct binary_expr : expr {
    std::shared_ptr<expr> lhs;
    token op;
    std::shared_ptr<expr> rhs;

    binary_expr(std::shared_ptr<expr> lhs, token t, std::shared_ptr<expr> rhs) : lhs{(lhs)}, op{t}, rhs{(rhs)} {}

    std::string debug() const override {
        return fmt::format("binary(left={}, operator={}, right={})", lhs->debug(), op, rhs->debug());
    }
};
struct logical_expr : expr {
    std::shared_ptr<expr> lhs;
    token op;
    std::shared_ptr<expr> rhs;

    logical_expr(std::shared_ptr<expr> lhs, token t, std::shared_ptr<expr> rhs) : lhs{(lhs)}, op{t}, rhs{(rhs)} {}
    std::string debug() const override {
        return fmt::format("logical(left={}, operand={}, right={})", lhs->debug(), op, rhs->debug());
    }
};
struct unary_expr : expr {
    token op;
    std::shared_ptr<expr> operand;

    unary_expr(token t, std::shared_ptr<expr> opr) : op{t}, operand{(opr)} {}

    std::string debug() const override {
        return fmt::format("unary(operator={}, operand={})", op, operand->debug());
    }
};
struct bool_expr : expr {
    bool value;
    bool_expr(bool x) : value{x} {}
    bool_expr(token tok) : value{tok == token::true_ ? true : false} {}

    std::string debug() const override {
        return fmt::format("bool({})", value);
    }
};
struct return_stmt : expr {
    std::shared_ptr<expr> value;
    return_stmt(std::shared_ptr<expr> v) : value{(v)} {}

    std::string debug() const override {
        return fmt::format("return({})", value->debug());
    }
};
struct num_expr : expr {
    std::int64_t value;
    num_expr(const std::string& value) : value{std::stoll(value)} {}
    num_expr(std::int64_t value) : value{value} {}

    std::string debug() const override {
        return fmt::format("number({})", value);
    }
};
struct array_expr : expr {
    std::vector<std::shared_ptr<expr>> elements;
    array_expr(const std::vector<std::shared_ptr<expr>>& e) : elements{e} {}
    std::string debug() const override {
        std::string str{"<"};
        for (std::size_t i = 0; i < elements.size(); ++i) {
            str.append(elements.at(i)->debug());
            if (i != elements.size() - 1) str.push_back(',');
        }
        str.push_back('>');
        return fmt::format("array({})'", str);
    }
};
struct ldouble_expr : expr {
    long double value;
    ldouble_expr(const std::string& value) : value{std::stold(value)} {}
    ldouble_expr(long double value) : value{value} {}

    std::string debug() const override {
        return fmt::format("float({})", value);
    }
};
struct string_expr : expr {
    std::string value;
    string_expr(const std::string& val) : value{val} {}

    std::string debug() const override {
        return fmt::format("string(\"{}\")", value);
    }
};
struct null_expr : expr {
    std::string debug() const override {
        return "null";
    }
};
struct break_stmt : expr {
    std::string debug() const override {
        return "break";
    }
};
struct continue_stmt : expr {
    std::string debug() const override {
        return "continue";
    }
};
struct self_expr : expr {
    std::string debug() const override {
        return "self";
    }
};
struct variable_expr : expr {
    std::string name;
    std::shared_ptr<expr> value;
    bool is_const;
    variable_expr(const std::string& val, std::shared_ptr<expr> v, bool i_c) : name{val}, value{(v)}, is_const{i_c} {}

    std::string debug() const override {
        return fmt::format("variable(name={}, value={})", name, value->debug());
    }
};
struct ident_expr : expr {
    std::string name;
    ident_expr(const std::string& val) : name{val} {}
    std::string debug() const override {
        return fmt::format("identifier({})", name);
    }
};
struct if_stmt : expr {
    std::shared_ptr<expr> condition;
    std::shared_ptr<expr> init;
    std::shared_ptr<expr> then_branch;
    std::shared_ptr<expr> else_branch;
    if_stmt(const std::shared_ptr<expr>& i, std::shared_ptr<expr> c, std::shared_ptr<expr> t, std::shared_ptr<expr> e)
        : condition{(c)}, init{i}, then_branch{(t)}, else_branch{(e)} {}

    std::string debug() const override {
        return fmt::format("if(confition={}, then={}, else={})", condition->debug(), then_branch->debug(),
                           (else_branch ? else_branch->debug() : "null"));
    }
};
struct call_expr : expr {
    std::shared_ptr<expr> callee;
    std::vector<std::shared_ptr<expr>> arguments;
    call_expr(std::shared_ptr<expr> c, std::vector<std::shared_ptr<expr>> a) : callee{(c)}, arguments{(a)} {}

    std::string debug() const override {
        std::string str{};
        if (arguments.size() == 0)
            str = "null";
        else
            for (auto& u : arguments) (str += u->debug()) += ',';
        return fmt::format("call(callee={}, arguments={})", callee->debug(), str);
    }
};
struct argument_expr : expr {
    std::string name;
    std::shared_ptr<expr> def;

    argument_expr(const std::string& name, const std::shared_ptr<expr>& d) : name{name}, def{d} {}

    std::string debug() const override {
        return fmt::format("argument(name={}, default={})", name, def ? def->debug() : "null");
    }
};
struct function_stmt : expr {
    std::string name;
    std::vector<std::shared_ptr<argument_expr>> arguments;
    std::vector<std::shared_ptr<expr>> body;

    function_stmt(const std::string& n, std::vector<std::shared_ptr<argument_expr>> a,
                  std::vector<std::shared_ptr<expr>> b)
        : name{n}, arguments{a}, body{b} {}

    std::string debug() const override {
        std::string args{};
        std::string bodystr{};
        for (auto& arg : arguments) (args += arg->debug()) += ',';
        for (auto& stmt : body) (bodystr += stmt->debug()) += ',';
        return fmt::format("fnc(name={}, arguments={}, body={})", name, args.size() > 0 ? args : "null", bodystr);
    }
};
struct for_stmt : expr {
    std::shared_ptr<expr> init;
    std::shared_ptr<expr> condition;
    std::shared_ptr<expr> branch;
    std::shared_ptr<expr> body;

    for_stmt(std::shared_ptr<expr> i, std::shared_ptr<expr> c, std::shared_ptr<expr> b, std::shared_ptr<expr> o)
        : init{i}, condition{c}, branch{b}, body{o} {}

    std::string debug() const override {
        return fmt::format("for(init={}, condition={}, branch={}, body={})", init->debug(), condition->debug(),
                           body->debug(), branch->debug());
    }
};
struct while_stmt : expr {
    std::shared_ptr<expr> init;
    std::shared_ptr<expr> branch;
    std::shared_ptr<expr> body;

    while_stmt(const std::shared_ptr<expr>& i, std::shared_ptr<expr> b, std::shared_ptr<expr> o)
        : init{i}, branch{(b)}, body{(o)} {}

    std::string debug() const override {
        return fmt::format("while(condition={}, body={})", branch->debug(), body->debug());
    }
};
struct class_expr : expr {
    std::string name;
    std::vector<std::shared_ptr<expr>> members;

    class_expr(const std::string& n, const std::vector<std::shared_ptr<expr>>& m) : name{n}, members{m} {}
    std::string debug() const override {
        return fmt::format("class({})", name);
    }
};
struct access_expr : expr {
    std::shared_ptr<expr> target;
    std::shared_ptr<expr> object;

    access_expr(const std::shared_ptr<expr>& t, const std::shared_ptr<expr>& e) : target{t}, object{e} {}

    std::string debug() const override {
        return fmt::format("access(target={}, sub={})", target->debug(), object->debug());
    }
};

struct block_stmt : expr {
    std::vector<std::shared_ptr<expr>> stmts;
    block_stmt(std::vector<std::shared_ptr<expr>> s) : stmts{(s)} {}

    std::string debug() const override {
        std::string out{};
        for (auto& stmt : stmts) { (out += stmt->debug()) += ';'; }
        return out.size() > 0 ? out : "empty";
    }
};
struct range_expr : expr {
    long double min_;
    long double max_;
    std::string debug() const override {
        return fmt::format("range(min={}, max={})", min_, max_);
    }
};
struct iterate_expr : expr {
    std::shared_ptr<expr> ident_;
    std::shared_ptr<expr> target;
    std::string debug() const override {
        return fmt::format("iterate(ident={}, target={})", ident_->debug(), target->debug());
    }
};
struct subscript_expr : expr {
    std::shared_ptr<expr> object;
    std::shared_ptr<expr> target;

    subscript_expr(const std::shared_ptr<expr>& o, const std::shared_ptr<expr>& t) : object{o}, target{t} {}

    std::string debug() const override {
        return fmt::format("subscript(object={}, target={})", object->debug(), target->debug());
    }
};
}  // namespace skai
#endif
