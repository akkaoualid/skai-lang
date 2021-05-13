#ifndef SKAI_LIBS_NATHS_hjkIeje83993
#define SKAI_LIBS_NATHS_hjkIeje83993
#include <fmt/format.h>

#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <skai/object.hpp>
#include <skai/utils.hpp>
#include <string>
#include <vector>
namespace skai {
namespace builtins {
template <class InterpreterClass>
struct print : object::callable<InterpreterClass> {
    std::size_t arity() override { return 1; }
    std::shared_ptr<object::object> call(InterpreterClass&,
                                         const std::vector<std::shared_ptr<object::object>>& args) override {
        for (const auto& arg : args) std::cout << arg->to_string() << ' ';
        return std::make_shared<object::null>();
    }
    bool variadic() const override { return true; }
    object_t to_underlying() const override { return object_t::function; }
    std::string to_string() const override { return "[pure function]"; }
};

template <class InterpreterClass>
struct println : object::callable<InterpreterClass> {
    std::size_t arity() override { return 1; }
    std::shared_ptr<object::object> call(InterpreterClass&,
                                         const std::vector<std::shared_ptr<object::object>>& args) override {
        std::cout << args.at(0)->to_string() << '\n';
        return std::make_shared<object::null>();
    }
    bool variadic() const override { return false; }
    object_t to_underlying() const override { return object_t::function; }
    std::string to_string() const override { return "[pure function]"; }
};

template <class InterpreterClass>
struct pow : object::callable<InterpreterClass> {
    std::size_t arity() override { return 2; }
    std::shared_ptr<object::object> call(InterpreterClass&,
                                         const std::vector<std::shared_ptr<object::object>>& args) override {
        if (args.at(0).get()->to_underlying() != object_t::integer &&
            args.at(1).get()->to_underlying() != object_t::integer)
            throw skai::exception{"'pow' expected integer arguments type"};
        return std::make_shared<object::integer>(
            std::pow(as<object::integer*>(args.at(0).get())->value, as<object::integer*>(args.at(1).get())->value));
    }

    bool variadic() const override { return false; }
    object_t to_underlying() const override { return object_t::function; }
    std::string to_string() const override { return "[pure function]"; }
};
template <class InterpreterClass>
struct prompt : object::callable<InterpreterClass> {
    std::size_t arity() override { return 1; }
    std::shared_ptr<object::object> call(InterpreterClass&,
                                         const std::vector<std::shared_ptr<object::object>>& args) override {
        if (args.at(0).get()->to_underlying() != object_t::string)
            throw skai::exception{"'prompt' expected string as a first argument"};
        std::string value;
        std::cout << as<object::string*>(args.at(0).get())->value;
        std::cin >> value;
        return std::make_shared<object::string>(value);
    }

    bool variadic() const override { return false; }
    object_t to_underlying() const override { return object_t::function; }
    std::string to_string() const override { return "[pure function]"; }
};
}  // namespace builtins
}  // namespace skai
#endif
