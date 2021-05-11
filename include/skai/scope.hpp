#ifndef SKAI_SCOPE_HPP_749393IEOEPEPE
#define SKAI_SCOPE_HPP_749393IEOEPEPE
#include <map>

#include "error.hpp"
namespace skai {
template <class ObjectClass>
struct scope {
    scope() : enclosing{nullptr}, contents{} {}

    scope(const scope& enc) : enclosing{new scope<ObjectClass>{enc}}, contents{} {}

    void define(const std::string& name, std::unique_ptr<ObjectClass> obj) { contents[name] = std::move(obj); }

    auto accessor(std::size_t depth) {
        auto env = *this;
        for (; depth >= 0; --depth) env = env.enclosing;
        return env;
    }

    void assing(const std::string& name, std::unique_ptr<ObjectClass> obj) {
        if (auto it = contents.find(name); it != contents.end()) {
            contents[name] = std::move(obj);
        } else if (enclosing) {
            enclosing->assign(name, std::move(obj));
        }
        throw skai::exception{fmt::format("use of ndeclared identifier {}.", name)};
    }

    auto get(const std::string& name) {
        if (auto it = contents.find(name); it != contents.end()) {
            return std::move(*it);
        } else if (enclosing) {
            return enclosing->get(name);
        }
        throw skai::exception{fmt::format("use of ndeclared identifier {}.", name)};
    }

   private:
    std::map<std::string, std::unique_ptr<ObjectClass>> contents;
    scope<ObjectClass>* enclosing;
};
}  // namespace skai
#endif
