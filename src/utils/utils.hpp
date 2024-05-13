#ifndef FALAFELS_UTILS_HPP
#define FALAFELS_UTILS_HPP

#include "string"
#include "vector"

// Trick to evaluate string at compile time. This permits us to use string comparison in switch statement.
constexpr unsigned int str2int(const char* str, int h = 0)
{
    return !str[h] ? 5381 : (str2int(str, h+1) * 33) ^ str[h];
}

bool replace_first(std::string& s, std::string const& toReplace, std::string const& replaceWith);
void replace_all(std::string& s, std::string const& toReplace, std::string const& replaceWith);

// Tool to use lambdas in std::visit (for std::variant), see: https://en.cppreference.com/w/cpp/utility/variant/visit
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

#endif //!FALAFELS_UTILS_HPP
