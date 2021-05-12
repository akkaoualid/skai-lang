#ifndef SKAI_SCOPE_HPP_749393IEOEPEPE
#define SKAI_SCOPE_HPP_749393IEOEPEPE
#include <map>
#include <memory>

#include "error.hpp"
namespace skai {
template <class ObjectClass>
struct scope {
    scope(const std::map<std::string, std::shared_ptr<ObjectClass>>& m = {}) : contents{m} {}

    scope(const scope<ObjectClass>& enc) : contents{}, enclosing{std::make_shared<scope<ObjectClass>>(enc.contents)} {}

    void define(const std::string& name, const std::shared_ptr<ObjectClass>& obj) { contents[name] = std::move(obj); }

    auto accessor(std::size_t depth) {
        auto env = *this;
        for (; depth >= 0; --depth) env = env.enclosing;
        return env;
    }

    void assign(const std::string& name, const std::shared_ptr<ObjectClass>& obj) {
        if (auto it = contents.find(name); it != contents.end()) {
            contents[name] = std::move(obj);
        } else if (enclosing) {
            enclosing->assign(name, std::move(obj));
        } else {
            throw skai::exception{fmt::format("use of undeclared identifier {}.", name)};
        }
    }

    auto get(const std::string& name) {
        if (auto it = contents.find(name); it != contents.end()) {
            return std::move(it->second);
        } else if (enclosing) {
            return enclosing->get(name);
        }
        throw skai::exception{fmt::format("use of undeclared identifier {}.", name)};
    }

   private:
    std::map<std::string, std::shared_ptr<ObjectClass>> contents;
    std::shared_ptr<scope<ObjectClass>> enclosing;
};
}  // namespace skai
#endif
