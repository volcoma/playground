#include "string_utils.h"

namespace string_utils
{
auto split(const std::string& text, char sep, bool skip_empty) -> std::vector<std::string>
{
	std::vector<std::string> tokens;
	std::size_t start = 0, end = 0;
	while((end = text.find(sep, start)) != std::string::npos)
	{
		std::string temp = text.substr(start, end - start);
		if(!temp.empty())
		{
			tokens.push_back(temp);
		}
		start = end + 1;
	}
	std::string temp = text.substr(start);
	if(skip_empty)
	{
		if(!temp.empty())
		{
			tokens.push_back(temp);
		}
	}
	else
	{
		tokens.push_back(temp);
	}

	return tokens;
}
} // namespace string_utils
