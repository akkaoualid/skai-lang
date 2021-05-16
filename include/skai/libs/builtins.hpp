#ifndef SKAI_LIBS_BUILTINS_hjkIeje83993
#define SKAI_LIBS_BUILTINS_hjkIeje83993
#include <fmt/format.h>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <memory>
#include <random>
#include <skai/object.hpp>
#include <skai/utils.hpp>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

namespace skai {
namespace builtins {

SK_FUNC(print, 1, 255, true, args) {
    for (const auto& arg : args) { std::printf("%s ", arg->to_string().c_str()); }
    std::putchar('\n');
    return std::make_shared<object::null>();
}
SK_FUNC_END

SK_FUNC(prompt, 1, 1, false, args) {
    auto str = dynamic_cast<object::string*>(args.at(0).get());
    if (!str) throw skai::exception{"'prompt' expected string as a first argument"};
    std::string value;
    std::cout << str->value;
    std::getline(std::cin, value);
    return std::make_shared<object::string>(value);
}
SK_FUNC_END

SK_FUNC(time, 0, 0, false, ) {
    return std::make_shared<object::ldouble>(std::chrono::system_clock::now().time_since_epoch().count() / 1000.0);
}
SK_FUNC_END

SK_FUNC(sleep, 1, 1, false, args) {
    auto amn = dynamic_cast<object::integer*>(args.at(0).get());
    if (!amn) throw skai::exception{"'sleep' expected integer as a first argument"};
    std::this_thread::sleep_for(std::chrono::milliseconds(amn->value));
    return std::make_shared<object::null>();
}
SK_FUNC_END

SK_FUNC(random, 2, 2, false, args) {
    std::random_device mt{};
    if (auto [f, l] = std::tuple{dynamic_cast<object::integer*>(args.at(0).get()),
                                 dynamic_cast<object::integer*>(args.at(1).get())};
        f && l) {
        if (l->value < f->value) throw skai::exception{"'random' invalid range provided"};
        std::uniform_int_distribution<std::int64_t> un(f->value, l->value);
        return std::make_shared<object::integer>(un(mt));
    }
    throw skai::exception{"random: expected integer types"};
}
SK_FUNC_END

SK_FUNC(type_of, 1, 1, false, args) {
    return std::make_shared<object::string>(args.at(0)->type_to_string());
}
SK_FUNC_END
}  // namespace builtins
}  // namespace skai
#endif
