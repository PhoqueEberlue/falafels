#include "string"
#include "vector"

std::vector<std::string> split_string(const std::string &txt, char ch);

// Trick to evaluate string at compile time. This permits us to use string comparison in switch statement.
constexpr unsigned int str2int(const char* str, int h = 0)
{
    return !str[h] ? 5381 : (str2int(str, h+1) * 33) ^ str[h];
}

