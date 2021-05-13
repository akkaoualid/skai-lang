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

    void define(const std::string& name, const std::shared_ptr<ObjectClass>& obj) {
        if (auto it = contents.find(name); it != contents.end()) {
            throw skai::exception{fmt::format("redefinition of '{}'", name)};
        }
        contents[name] = obj;
    }

    const auto& get_contents() const { return contents; }
    void set_contents(const std::map<std::string, std::shared_ptr<ObjectClass>>& cn) { contents = cn; }

    auto accessor(std::size_t depth) {
        auto env = *this;
        for (; depth >= 0; --depth) env = env.enclosing;
        return env;
    }

    void assign(const std::string& name, const std::shared_ptr<ObjectClass>& obj) {
        if (auto it = contents.find(name); it != contents.end()) {
            contents[name] = obj;
        } else if (enclosing) {
            enclosing->assign(name, obj);
        } else {
            throw skai::exception{fmt::format("use of undeclared identifier {}.", name)};
        }
    }

    auto get(const std::string& name) {
        if (auto it = contents.find(name); it != contents.end()) {
            return it->second;
        } else if (enclosing) {
            return enclosing->get(name);
        }
        throw skai::exception{fmt::format("use of undeclared identifier {}.", name)};
    }

    std::string to_string() const {
        std::string cnt;
        for (const auto& [key, value] : contents) {
            cnt += fmt::format("{} | {}\n", key, value->to_string());
        }
        if (enclosing != nullptr) (cnt += " ->") += enclosing->to_string();
        return cnt;
    }

   private:
    std::map<std::string, std::shared_ptr<ObjectClass>> contents;
    std::shared_ptr<scope<ObjectClass>> enclosing;
};
}  // namespace skai
#endif
