#ifndef SKAI_OBJECT_HPP_473893KEIEOE
#define SKAI_OBJECT_HPP_473893KEIEOE
#include <fmt/format.h>

#include <memory>
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
template <class InterpreterClass>
struct callable : object {
    virtual std::size_t arity() = 0;
    virtual std::shared_ptr<object> call(InterpreterClass&, const std::vector<std::shared_ptr<object>>& args) = 0;
    // object_t to_underlying() const override { return object_t::callable; }
    virtual ~callable() {}
};

template <class InterpreterClass>
struct function : callable<InterpreterClass> {
    function(function_stmt* fnc, scope<object>& s) : decl{fnc}, env{s} {}
    std::string to_string() const override { return fmt::format("[function '{}']", decl->name); }
    std::size_t arity() override { return decl->arguments.size(); }

    std::shared_ptr<object> call(InterpreterClass& inter, const std::vector<std::shared_ptr<object>>& args) override {
        for (std::size_t i = 0; i < arity(); ++i) {
            env.define(static_cast<ident_expr*>(decl->arguments.at(i).get())->name, std::move(args.at(i)));
        }

        inter.m_exec_block(std::move(decl->body), env);
        return inter.get_return();
    }

    object_t to_underlying() const override { return object_t::function; }

   private:
    function_stmt* decl;
    scope<object> env;
};
struct boolean : object {
    bool value;
    boolean(bool v) : value{v} {}

    object_t to_underlying() const override { return object_t::boolean; }
    std::string to_string() const override { return fmt::format("{}", value); }
};
struct integer : object {
    int value;
    integer(int v) : value{v} {}

    object_t to_underlying() const override { return object_t::integer; }
    std::string to_string() const override { return fmt::format("{}", value); }
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
struct null : object {
    object_t to_underlying() const override { return object_t::null; }
    std::string to_string() const override { return "null"; }
};
}  // namespace object
}  // namespace skai
#endif
