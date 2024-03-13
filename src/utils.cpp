#include "node.hpp"

// Yeeted from stackoverflow 🙃
std::vector<std::string> split_string(const std::string &txt, char ch)
{
    auto string_vec = std::vector<std::string>();

    size_t pos = txt.find(ch);
    size_t initialPos = 0;

    // Decompose statement
    while( pos != std::string::npos ) {
        string_vec.push_back(txt.substr(initialPos, pos - initialPos));
        initialPos = pos + 1;

        pos = txt.find(ch, initialPos);
    }

    // Add the last one
    string_vec.push_back(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1));

    return string_vec;
}
