#include <fstream>
#include <fmt/core.h>
#include <skai/interpreter.hpp>
#include <skai/lexer.hpp>
#include <skai/parser.hpp>
#include <string>

int main(int, char** argv) {
    std::string input;
    std::string filename;
    if (std::string{argv[1]} == "-e") {
        input = argv[2];
        filename = "argv";
    } else {
        std::ifstream file{argv[1]};
        filename = argv[1];
        std::string tmp;
        while (std::getline(file, tmp)) (input += tmp) += '\n';
    }
    try {
        skai::lexer lexer{input, filename};
        skai::parser parse{lexer.lex(), filename};
        skai::interpreter inter;
        auto o = parse.parse();
        inter.interpret(o);
    } catch (skai::exception& exc) { fmt::print("{}\n", exc.msg); }
}
