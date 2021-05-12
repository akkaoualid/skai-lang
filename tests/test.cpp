#include <fstream>
#include <iostream>
#include <string>

#include "interpreter.hpp"
#include "lexer.hpp"
#include "parser.hpp"

int main(int argc, char** argv) {
    try {
        std::ifstream file{argv[1]};
        std::string temp;
        std::string full;
        while (std::getline(file, temp)) (full += temp) += '\n';
        skai::lexer lexer{full, argv[1]};
        auto lexed = lexer.lex();
        skai::parser parser{lexed, argv[1]};
        skai::interpreter inter{};
        auto out = parser.parse();
        std::cout << '\n';
        std::cout << full << "\n\n";
        inter.interpret(std::move(out));
    } catch (skai::exception exc) {
        std::cout << exc.msg << '\n';
    }
}
