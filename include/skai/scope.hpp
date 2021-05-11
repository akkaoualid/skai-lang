#ifndef SKAI_SCOPE_HPP_749393IEOEPEPE
#define SKAI_SCOPE_HPP_749393IEOEPEPE
#include <map>
#include <optional>

#include "ast.hpp"
#include "error.hpp"
namespace skai {
struct scope {
    scope() : enclosing{std::nullopt}, contents{} {}

    scope(const scope& enc) : enclosing{enc}, contents{} {}

    void define(const std::string& name, std::unique_ptr<expr> obj) { contents[name] = std::move(obj); }

    auto accessor(std::size_t depth) {
        auto env = *this;
        for (; depth >= 0; --depth) env = env.enclosing;
        return env;
    }

    void assing(const std::string& name, std::unique_ptr<expr> obj) {
        if (auto it = contents.find(name); it != contents.end()) {
            contents[name] = std::move(obj);
        } else if (enclosing.has_value()) {
            enclosing.assign(name, std::move(obj));
        }
        throw skai::exception{fmt::format("use of ndeclared identifier {}.", name)};
    }

    auto get(const std::string& name) {
        if (auto it = contents.find(name); it != contents.end()) {
            return std::move(*it);
        } else if (enclosing.has_value()) {
            return enclosing.get(name);
        }
        throw skai::exception{fmt::format("use of ndeclared identifier {}.", name)};
    }

   private:
    std::map<std::string, std::unique_ptr<expr>> contents;
    std::optional<scope> enclosing;
};
}  // namespace skai
#endif
