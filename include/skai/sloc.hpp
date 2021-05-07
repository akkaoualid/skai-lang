#ifndef SKAI_SLOC_HPP_7493030303
#define SKAI_SLOC_HPP_7493030303
#include <cstdint>
#include <string>
namespace skai {
struct source_location {
    std::string file;
    std::uint64_t line;
    std::uint64_t column;
};
}  // namespace skai
#endif
