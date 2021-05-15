#ifndef SKAI_ERROR_HPP_73290203
#define SKAI_ERROR_HPP_73290203
#include <fmt/format.h>

#include <string>

#include "sloc.hpp"
namespace skai {
struct exception {
    std::string msg;
};
struct error_info {
    std::string message;
    std::string file;
    std::string function;
    std::string module_;
    std::size_t lineno;
    std::size_t colno;
    exception* type;
};
}  // namespace skai
#endif
