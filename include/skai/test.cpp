#include <iostream>

#include "lexer.hpp"
#include "parser.hpp"

int main() {
    skai::lexer l{"5 / 10", "source"};
    auto parsed = skai::parser{l.lex(), "source"};
    parsed.parse();
}
