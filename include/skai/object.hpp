#ifndef SKAI_OBJECT_HPP_473893KEIEOE
#define SKAI_OBJECT_HPP_473893KEIEOE
#include <fmt/format.h>

#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "ast.hpp"
#include "scope.hpp"

namespace skai {
namespace object {
struct object {
    virtual std::string to_string() const = 0;
    virtual std::string type_to_string() const = 0;
#define DEF_OPER(oper)                                                                                \
    virtual std::shared_ptr<object> operator oper(const std::shared_ptr<object>& o) {                 \
        throw skai::exception{fmt::format("invalid operand for token '{}', between {} and {}", #oper, \
                                          type_to_string(), o->type_to_string())};                    \
    }

    DEF_OPER(+)
    DEF_OPER(-)
    DEF_OPER(+=)
    DEF_OPER(-=)
    DEF_OPER(*)
    DEF_OPER(/)
    DEF_OPER(*=)
    DEF_OPER(/=)
    DEF_OPER(&)
    DEF_OPER(^)
    DEF_OPER(|)
    DEF_OPER(&=)
    DEF_OPER(^=)
    DEF_OPER(%)
    DEF_OPER(%=)
    DEF_OPER(|=)
    DEF_OPER(>>)
    DEF_OPER(<<)
    DEF_OPER(>>=)
    DEF_OPER(<<=)
    DEF_OPER(==)
    DEF_OPER(!=)
    DEF_OPER(&&)
    DEF_OPER(||)
    DEF_OPER(<)
    DEF_OPER(>)
    DEF_OPER(<=)
    DEF_OPER(>=)
    DEF_OPER([])
    virtual ~object() {}
};
using arg_t = std::vector<std::shared_ptr<object>>;
struct null : object {
    std::string to_string() const override {
        return "null";
    }
    std::string type_to_string() const override {
        return "null";
    }
};
template <class InterpreterClass>
struct callable : object {
    /*
     */
    virtual std::size_t mina() = 0;
    virtual std::size_t maxa() = 0;
    virtual bool variadic() const = 0;
    virtual std::shared_ptr<object> call(InterpreterClass&, const std::vector<std::shared_ptr<object>>& args) = 0;
    virtual ~callable() {}
};

#define SK_FUNC(name, arg1, arg2, var, args)                          \
    template <class InterpreterClass>                                 \
    struct name : object::callable<InterpreterClass> {                \
        std::size_t mina() override {                                 \
            return arg1;                                              \
        }                                                             \
        std::size_t maxa() override {                                 \
            return arg2;                                              \
        }                                                             \
        bool variadic() const override {                              \
            return var;                                               \
        }                                                             \
        std::string to_string() const override {                      \
            return "[pure function]";                                 \
        }                                                             \
        std::string type_to_string() const override {                 \
            return "function";                                        \
        }                                                             \
        std::shared_ptr<object::object> call(InterpreterClass& inter, \
                                             const std::vector<std::shared_ptr<object::object>>& args) override

// clang-format off
#define SK_FUNC_END };
// clang-format on

template <class InterpreterClass>
struct module_base : object {
    virtual std::vector<std::shared_ptr<object>> objects() = 0;
    virtual ~module_base() {}
};
template <class InterpreterClass>
struct function : callable<InterpreterClass> {
    function(function_stmt fnc, const scope<object>& s, bool init = false, bool v = false)
        : decl{fnc}, env{s.get_contents()}, variadic_{v}, is_init{init} {}

    std::string to_string() const override {
        return fmt::format("[function '{}']", decl.name);
    }

    std::string type_to_string() const override {
        return "function";
    }
    std::size_t mina() override {
        return std::count_if(decl.arguments.begin(), decl.arguments.end(),
                             [](const auto& s) { return s->def == nullptr; });
    }

    std::size_t maxa() override {
        return decl.arguments.size();
    }

    std::shared_ptr<object> call(InterpreterClass& inter, const std::vector<std::shared_ptr<object>>& args) override {
        for (std::size_t i = 0; i < maxa(); ++i) {
            try {
                env.define(decl.arguments.at(i)->name, args.at(i));
            } catch (std::out_of_range) {
                env.define(decl.arguments.at(i)->name, inter.m_eval(decl.arguments.at(i)->def));
            }
        }

        inter.set_in_func(true);
        inter.m_exec_block(decl.body, env, true);
        inter.set_in_func(false);
        auto ret = inter.get_return();
        if (auto in = dynamic_cast<null*>(ret.get()); is_init)
            throw skai::exception{"constructors can't return anything"};

        inter.set_ret(std::make_shared<null>());
        if (is_init) return env.get("self");
        return ret;
    }

    template <class Instance>
    auto bind(Instance* ins) {
        auto e{env};
        e.define("self", ins);
        return std::make_shared<function<InterpreterClass>>(decl, e, is_init);
    }

    bool variadic() const override {
        return variadic_;
    }

    function_stmt decl;
    scope<object> env;
    std::map<std::string, std::function<std::shared_ptr<object>(const std::vector<std::shared_ptr<object>>&)>> members;
    bool variadic_;
    bool is_init;
};

#define ADD_OP(ret, op, arg)                                                                              \
    std::shared_ptr<object> operator op(const std::shared_ptr<object>& obj) override {                    \
        if (auto v = dynamic_cast<arg*>(obj.get())) { return std::make_shared<ret>(value op v->value); }  \
        throw skai::exception{fmt::format("invalid operand for binary operator '{}', '{}' and '{}'", #op, \
                                          type_to_string(), obj->type_to_string())};                      \
    }
#define BIND_OP(ret, cxxfn, arg, op)                                                                               \
    std::shared_ptr<object> operator op(const std::shared_ptr<object>& obj) override {                             \
        if (auto v = dynamic_cast<arg*>(obj.get())) { return std::make_shared<ret>(std::cxxfn(value, v->value)); } \
        throw skai::exception{fmt::format("invalid operand for binary operator '{}', '{}' and '{}'", #op,          \
                                          type_to_string(), obj->type_to_string())};                               \
    }
struct boolean : object {
    bool value;
    boolean(bool v) : value{v} {}

    std::string to_string() const override {
        return fmt::format("{}", value);
    }
    std::string type_to_string() const override {
        return "boolean";
    }
    ADD_OP(boolean, ==, boolean)
    ADD_OP(boolean, !=, boolean)
    ADD_OP(boolean, &&, boolean)
    ADD_OP(boolean, ||, boolean)
};
struct ldouble : object {
    long double value;
    ldouble(long double v) : value{v} {}

    std::string to_string() const override {
        std::ostringstream ss;
        ss << value;
        return ss.str();
    }
    std::string type_to_string() const override {
        return "float";  // kekw
    }
    ADD_OP(ldouble, +, ldouble)
    ADD_OP(ldouble, -, ldouble)
    ADD_OP(ldouble, /, ldouble)
    ADD_OP(ldouble, *, ldouble)
    BIND_OP(ldouble, fmod, ldouble, %)
    ADD_OP(boolean, ==, ldouble)
    ADD_OP(boolean, !=, ldouble)
    ADD_OP(boolean, <, ldouble)
    ADD_OP(boolean, >, ldouble)
    ADD_OP(boolean, <=, ldouble)
    ADD_OP(boolean, >=, ldouble)
};
struct integer : object {
    std::int64_t value;
    integer(std::int64_t v) : value{v} {}

    std::string to_string() const override {
        return fmt::format("{}", value);
    }

    std::string type_to_string() const override {
        return "integer";
    }
    ADD_OP(integer, +, integer)
    ADD_OP(integer, -, integer)
    ADD_OP(ldouble, /, integer)
    ADD_OP(integer, *, integer)
    ADD_OP(integer, ^, integer)
    ADD_OP(integer, |, integer)
    ADD_OP(integer, &, integer)
    ADD_OP(integer, %, integer)
    ADD_OP(integer, >>, integer)
    ADD_OP(integer, <<, integer)
    ADD_OP(boolean, ==, integer)
    ADD_OP(boolean, !=, integer)
    ADD_OP(boolean, <, integer)
    ADD_OP(boolean, >, integer)
    ADD_OP(boolean, <=, integer)
    ADD_OP(boolean, >=, integer)
};
struct string : object {
    std::string value;
    string(std::string v) : value{v} {}

    std::string to_string() const override {
        std::string str;
        for (std::size_t i = 0; i < value.size(); ++i) {
            if (value.at(i) == '\\') {
                switch (char c = value.at(i + 1)) {
                    default:
                        throw skai::exception{fmt::format("invalid escape character '{}'", c)};
                    case 'n':
                        str.push_back('\n');
                        break;
                    case 't':
                        str.push_back('\t');
                        break;
                    case 'r':
                        str.push_back('\r');
                        break;
                    case '\\':
                        str.push_back('\\');
                        break;
                    case '"':
                        str.push_back('"');
                        break;
                    case 'b':
                        str.push_back('\b');
                        break;
                    case 'v':
                        str.push_back('\v');
                        break;
                    case 'f':
                        str.push_back('\f');
                        break;
                    case '0':
                        str.push_back('\0');
                        break;
                }
                i += 2;
            } else {
                str.push_back(value.at(i));
            }
        }
        return str;
    }
    std::string type_to_string() const override {
        return "string";
    }
    ADD_OP(string, +, string)
    ADD_OP(boolean, ==, string)
    ADD_OP(boolean, !=, string)
    ADD_OP(boolean, <, string)
    ADD_OP(boolean, >, string)
    ADD_OP(boolean, <=, string)
    ADD_OP(boolean, >=, string)

    std::shared_ptr<object> operator[](const std::shared_ptr<object>& idx) override {
        if (auto i = dynamic_cast<integer*>(idx.get())) {
            std::size_t start = 0;
            if (i->value < 0) start = value.size();
            try {
                return std::make_shared<string>(std::string(1, value.at(start + i->value)));
            } catch (std::out_of_range) { throw skai::exception{fmt::format("out of bounds index '{}'", i->value)}; }
        }
        throw skai::exception{
            fmt::format("expected type integer in string subscript operator got '{}' instead", idx->type_to_string())};
    }
};
struct array : object {
    std::vector<std::shared_ptr<object>> values;

    array(const std::vector<std::shared_ptr<object>>& v) : values{v} {}

    std::string to_string() const override {
        std::string full{"["};
        for (std::size_t i = 0; i < values.size(); ++i) {
            full += values.at(i)->to_string();
            if (i != values.size() - 1) full += ',';
        }
        full += ']';
        return full;
    }
    std::string type_to_string() const override {
        return "array";
    }
    std::shared_ptr<object> operator[](const std::shared_ptr<object>& idx) override {
        if (auto i = dynamic_cast<integer*>(idx.get())) {
            std::size_t start = 0;
            if (i->value < 0) start = values.size();
            try {
                return values.at(start + i->value);
            } catch (std::out_of_range) { throw skai::exception{fmt::format("out of bounds index '{}'", i->value)}; }
        }
        throw skai::exception{
            fmt::format("expected type integer in array subscript operator got '{}' instead", idx->type_to_string())};
    }
};
struct variable : object {
    std::string name;
    bool is_const;
    std::shared_ptr<object> value;

    variable(const std::string& n, bool i, const std::shared_ptr<object>& v) : name{n}, is_const{i}, value{v} {}

    std::string to_string() const override {
        return value->to_string();
    }
    std::string type_to_string() const override {
        return value->type_to_string();
    }
#define VAR_ADD_OP_RET(op)                                                             \
    std::shared_ptr<object> operator op(const std::shared_ptr<object>& obj) override { \
        if (auto v = dynamic_cast<variable*>(obj.get()))                               \
            return value.get()->operator op(v->value);                                 \
        else                                                                           \
            return value.get()->operator op(obj);                                      \
    }
#define VAR_ADD_OP(op)                                                                    \
    std::shared_ptr<object> operator op##=(const std::shared_ptr<object>& obj) override { \
        if (auto v = dynamic_cast<variable*>(obj.get()))                                  \
            value = value.get()->operator op(v->value);                                   \
        else                                                                              \
            value = value.get()->operator op(obj);                                        \
        return value;                                                                     \
    }                                                                                     \
    VAR_ADD_OP_RET(op)

    VAR_ADD_OP(+)
    VAR_ADD_OP(-)
    VAR_ADD_OP(/)
    VAR_ADD_OP(*)
    VAR_ADD_OP(^)
    VAR_ADD_OP(|)
    VAR_ADD_OP(>>)
    VAR_ADD_OP(<<)
    VAR_ADD_OP_RET(>)
    VAR_ADD_OP_RET(>=)
    VAR_ADD_OP_RET(<)
    VAR_ADD_OP_RET(<=)
    VAR_ADD_OP(&)
    VAR_ADD_OP(%)
};

}  // namespace object
}  // namespace skai
#endif
