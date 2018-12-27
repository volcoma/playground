#include "string_utils.h"
#include <cstring>

namespace string_utils
{

auto tokenize(const std::string& str, const std::string& delimiters) -> std::vector<std::string>
{
    std::vector<std::string> tokens;
    // Skip delimiters at beginning.
    auto last_pos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    auto pos = str.find_first_of(delimiters, last_pos);

    while (std::string::npos != pos || std::string::npos != last_pos)
    {
        // Found a token, add it to the vector.
        tokens.emplace_back(str.substr(last_pos, pos - last_pos));
        // Skip delimiters.  Note the "not_of"
        last_pos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, last_pos);
    }

    return tokens;
}

} // namespace string_utils
