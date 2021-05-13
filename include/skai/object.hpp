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
enum class object_t { object, callable, function, boolean, integer, ldouble, string, null, builtin };
namespace object {
struct object {
    virtual std::string to_string() const = 0;
    virtual object_t to_underlying() const = 0;
    virtual ~object() {}
};
struct null : object {
    object_t to_underlying() const override { return object_t::null; }
    std::string to_string() const override { return "null"; }
};
template <class InterpreterClass>
struct callable : object {
    virtual std::size_t arity() = 0;
    virtual bool variadic() const = 0;
    virtual std::shared_ptr<object> call(InterpreterClass&, const std::vector<std::shared_ptr<object>>& args) = 0;
    virtual ~callable() {}
};
template <class InterpreterClass>
struct module_base : object {
    virtual std::vector<std::shared_ptr<object>> objects() = 0;
    virtual ~module_base() {}
};
template <class InterpreterClass>
struct function : callable<InterpreterClass> {
    function(function_stmt fnc, const scope<object>& s, bool v = false)
        : decl{fnc}, env{s.get_contents()}, variadic_{v} {}
    std::string to_string() const override { return fmt::format("[function '{}']", decl.name); }
    std::size_t arity() override { return decl.arguments.size(); }

    std::shared_ptr<object> call(InterpreterClass& inter, const std::vector<std::shared_ptr<object>>& args) override {
        for (std::size_t i = 0; i < arity(); ++i) {
            env.define(static_cast<ident_expr*>(decl.arguments.at(i).get())->name, args.at(i));
        }

        inter.set_in_func(true);
        inter.m_exec_block(decl.body, env, true);
        inter.set_in_func(false);
        auto ret = inter.get_return();
        inter.set_ret(std::make_shared<null>());
        return ret;
    }

    bool variadic() const override { return variadic_; }

    object_t to_underlying() const override { return object_t::function; }

   private:
    function_stmt decl;
    scope<object> env;
    bool variadic_;
};
struct boolean : object {
    bool value;
    boolean(bool v) : value{v} {}

    object_t to_underlying() const override { return object_t::boolean; }
    std::string to_string() const override { return fmt::format("{}", value); }
};
struct integer : object {
    std::int64_t value;
    integer(std::int64_t v) : value{v} {}

    object_t to_underlying() const override { return object_t::integer; }
    std::string to_string() const override {
        std::ostringstream ss;
        ss << value;
        return ss.str();
    }
};
struct ldouble : object {
    long double value;
    ldouble(long double v) : value{v} {}

    object_t to_underlying() const override { return object_t::ldouble; }
    std::string to_string() const override { return fmt::format("{}", value); }
};
struct string : object {
    std::string value;
    string(std::string v) : value{v} {}

    object_t to_underlying() const override { return object_t::string; }
    std::string to_string() const override { return value; }
};
}  // namespace object
}  // namespace skai
#endif
