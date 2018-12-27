#pragma once
#include <string>
#include <vector>

namespace string_utils
{
auto tokenize(const std::string& text, const std::string& delimiters) -> std::vector<std::string>;

}
