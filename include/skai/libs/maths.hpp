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
SK_FUNC(abs, 1, 1, false) {
    if (auto inner = dynamic_cast<object::integer*>(args.at(0).get()))
        return std::make_shared<object::integer>(std::abs(inner->value));
    if (auto inner = dynamic_cast<object::ldouble*>(args.at(0).get()))
        return std::make_shared<object::ldouble>(std::abs(inner->value));
    throw skai::exception{"'abs' expected arguments of type int/float"};
}
SK_FUNC_END
}  // namespace builtins
}  // namespace skai
#endif
