#ifndef SKAI_OBJECT_HPP_473893KEIEOE
#define SKAI_OBJECT_HPP_473893KEIEOE
#include <fmt/format.h>

#include <memory>
#include <string>
#include <vector>

#include "ast.hpp"
#include "scope.hpp"

namespace skai {
namespace object {
struct object {
    virtual std::string to_string() const = 0;
    virtual ~object() {}
};
template <class InterpreterClass>
struct callable : object {
    virtual std::size_t arity() = 0;
    virtual std::unique_ptr<object> call(InterpreterClass&, std::vector<std::unique_ptr<object>> args) = 0;
    virtual ~callable() {}
};

template <class InterpreterClass>
struct function : callable<InterpreterClass> {
    function(function_stmt* fnc, scope<object> const& s) : decl{fnc}, env{s} {}
    std::string to_string() const override { return fmt::format("{{function '{}'}}", decl->name); }
    std::size_t arity() override { return decl->arguments.size(); }

    std::unique_ptr<object> call(InterpreterClass& inter, std::vector<std::unique_ptr<object>> args) override {
        scope<InterpreterClass> env_{env};
        for (std::size_t i = 0; i < arity(); ++i) {
            env_.define(decl->arguments.at(i), args.at(i));
        }

        return inter.get_return;
    }

   private:
    function_stmt* decl;
    scope<InterpreterClass> env;
};
struct boolean : object {
    bool value;
    boolean(bool v) : value{v} {}

    std::string to_string() const override { return fmt::format("{}", value); }
};
struct integer : object {
    int value;
    integer(int v) : value{v} {}

    std::string to_string() const override { return fmt::format("{}", value); }
};
struct ldouble : object {
    long double value;
    ldouble(long double v) : value{v} {}

    std::string to_string() const override { return fmt::format("{}", value); }
};
struct string : object {
    std::string value;
    string(std::string v) : value{v} {}

    std::string to_string() const override { return value; }
};
struct null : object {
    std::string to_string() const override { return "null"; }
};
}  // namespace object
}  // namespace skai
#endif
