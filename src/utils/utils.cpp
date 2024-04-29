#include <vector>
#include <string>

#include "../node/node.hpp"

// Source: https://stackoverflow.com/questions/5878775/how-to-find-and-replace-string
bool replace_first(std::string& s, std::string const& toReplace, std::string const& replaceWith) 
{
    std::size_t pos = s.find(toReplace);
    if (pos == std::string::npos) return false;
    s.replace(pos, toReplace.length(), replaceWith);
    return true;
}

void replace_all(std::string& s, std::string const& toReplace, std::string const& replaceWith) 
{
    while (replace_first(s, toReplace, replaceWith));
}
