#ifndef SKAI_LEXER_HPP_73399393
#define SKAI_LEXER_HPP_73399393
#include <fmt/format.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "error.hpp"
#include "sloc.hpp"
namespace skai {
enum class token {
    eq,
    d_eq,
    gt,
    gt_eq,
    lt,
    lt_eq,
    not_,
    not_eq_,
    and_,
    or_,
    if_,
    else_,
    for_,
    while_,
    fun,
    break_,
    continue_,
    class_,
    null,
    false_,
    true_,
    let,
    this_,
    return_,
    const_,
    of,
    scolon,
    colon,
    comma,
    dot,
    plus,
    minus,
    star,
    slash,
    mod,
    xor_,
    b_or,
    lshift,
    rshift,
    b_and,
    arrow,
    lparen,
    rparen,
    lbracket,
    rbracket,
    lcbracket,
    rcbracket,
    string,
    identifier,
    number,
    range
};

struct token_handler {
    token token;
    std::string str;
    skai::source_location loc;
    template <class... tok>
    bool is(tok... t) {
        return (... || (token == t));
    }

    template <class... tok>
    bool isnot(tok... t) {
        return !is(t...);
    }
};

static std::unordered_map<std::string, token> keywords{
    {"and", token::and_},       {"or", token::or_},     {"if", token::if_},       {"const", token::const_},
    {"fun", token::fun},        {"let", token::let},    {"class", token::class_}, {"while", token::while_},
    {"for", token::for_},       {"else", token::else_}, {"break", token::break_}, {"continue", token::continue_},
    {"return", token::return_}, {"true", token::true_}, {"false", token::false_}, {"of", token::of},
    {"null", token::null},
};
struct lexer {
    lexer(const std::string& str, const std::string& name) : m_inp{str}, m_file{name} {}

    void scan() {
        switch (m_get()) {
            default:
                if (std::isdigit(static_cast<unsigned char>(m_get()))) {
                    m_number();
                } else if (std::isalpha(static_cast<unsigned char>(m_get()))) {
                    m_ident();
                }
                break;
            case ')':
                m_addtok(token::rparen);
                break;
            case '(':
                m_addtok(token::lparen);
                break;
            case '}':
                m_addtok(token::rbracket);
                break;
            case '{':
                m_addtok(token::lbracket);
                break;
            case ']':
                m_addtok(token::rcbracket);
                break;
            case '[':
                m_addtok(token::lcbracket);
                break;
            case '.':
                m_addtok(token::dot);
                break;
            case ',':
                m_addtok(token::comma);
                break;
            case ';':
                m_addtok(token::scolon);
                break;
            case ':':
                m_addtok(token::colon);
                break;
            case '*':
                m_addtok(token::star);
                break;
            case '+':
                m_addtok(token::plus);
                break;
            case '%':
                m_addtok(token::mod);
                break;
            case ' ':
            case '\t':
            case '\r':
                break;
            case '\n':
                m_line++;
                break;
            case '^':
                m_addtok(token::xor_);
                break;
            case '&':
                if (m_peek() == '&') {
                    m_addtok(token::and_, "&&");
                    m_advance();
                } else {
                    m_addtok(token::b_and);
                }
                break;
            case '|':
                if (m_peek() == '|') {
                    m_addtok(token::or_, "||");
                    m_advance();
                } else {
                    m_addtok(token::b_or);
                }
                break;
            case '-':
                if (m_peek() == '>') {
                    m_addtok(token::arrow, "->");
                    m_advance();
                } else {
                    m_addtok(token::minus);
                }
                break;
            case '/':
                if (m_peek() == '/') {
                    m_advance();
                    while (m_get() != '\n') m_advance();
                }
                m_addtok(token::slash);
                break;
            case '!':
                if (m_peek() == '=') {
                    m_addtok(token::not_eq_, "!=");
                    m_advance();
                } else {
                    m_addtok(token::not_);
                }
                break;
            case '=':
                if (m_peek() == '=') {
                    m_addtok(token::d_eq, "==");
                    m_advance();
                } else {
                    m_addtok(token::eq);
                }
                break;
            case '<':
                if (m_peek() == '=') {
                    m_addtok(token::lt_eq, "<=");
                    m_advance();
                } else if (m_peek() == '<') {
                    m_addtok(token::lshift, "<<");
                    m_advance();
                } else {
                    m_addtok(token::lt);
                }
                break;
            case '>':
                if (m_peek() == '=') {
                    m_addtok(token::gt_eq, ">=");
                    m_advance();
                } else if (m_peek() == '>') {
                    m_addtok(token::rshift, ">>");
                    m_advance();
                } else {
                    m_addtok(token::gt);
                }
                break;
            case '"':
                m_advance();
                m_string();
                break;
        }
        m_advance();
    }

    auto lex() {
        while (!at_end()) {
            scan();
        }
        return m_out;
    }

   private:
    void m_advance(std::size_t x = 1) { m_pos += x; }
    char m_peek(std::size_t x = 1) { return ((m_pos + x) >= m_inp.size() ? '\0' : m_inp.at(m_pos + x)); }
    char m_previous(std::size_t x = 1) { return m_peek(m_pos - x); }
    bool at_end() { return m_pos >= m_inp.size(); }
    char m_get() { return (at_end() ? '\0' : m_inp.at(m_pos)); }
    void m_addtok(token tok) { m_out.push_back(token_handler{tok, std::string(1, m_get()), {m_file, m_line, m_pos}}); }
    void m_addtok(token tok, std::string str) { m_out.push_back(token_handler{tok, str, {m_file, m_line, m_pos}}); }
    void m_string() {
        std::string full;
        while (m_get() != '"' && !at_end()) {
            full.push_back(m_get());
            m_advance();
        }
        if (at_end()) {
            throw skai::exception{"unterminated string literal '\"'"};
        }
        m_addtok(token::string, full);
    }
    void m_number() {
        std::string full;
        while (std::isdigit(static_cast<unsigned char>(m_get())) || m_get() == '.') {
            full.push_back(m_get());
            m_advance();
        }
        m_addtok(token::number, full);
        m_advance(-1);
    }

    void m_ident() {
        std::string full;
        while (std::isdigit(static_cast<unsigned char>(m_get())) || m_alph(m_get())) {
            full.push_back(m_get());
            m_advance();
        }
        auto key = keywords.find(full);
        if (key != keywords.end()) {
            m_addtok(key->second, full);
        } else {
            m_addtok(token::identifier, full);
        }
        m_advance(-1);
    }

    bool m_alph(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_') || (c == '\''); }

    std::vector<token_handler> m_out;
    std::size_t m_line = 1;
    std::size_t m_pos = 0;
    const std::string m_inp;
    const std::string m_file;
};
}  // namespace skai
#endif
