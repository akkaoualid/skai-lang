#include <iostream>

#include "interpreter.hpp"
#include "lexer.hpp"
#include "parser.hpp"

int main() {
    skai::lexer l{R"(
fun fact(x) {
    if x <= 0 {
        return 1;
    } else {
        return x*fact(x-1);
    }
}
            )",
                  "source"};
    auto lexed = l.lex();
    for (auto elm : lexed) std::cout << elm.str << ' ';
    auto parsed = skai::parser{lexed, "source"};
    std::cout << '\n';
    try {
        auto out = parsed.parse();
        for (auto&& elm : out) {
            std::cout << elm->debug() << ' ';
        }
    } catch (skai::exception& exc) {
        std::cout << exc.msg;
    }
}
