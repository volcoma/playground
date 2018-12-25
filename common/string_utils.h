#pragma once
#include <string>
#include <vector>

namespace string_utils
{
auto split(const std::string& text, char sep, bool skip_empty = true) -> std::vector<std::string>;
}
