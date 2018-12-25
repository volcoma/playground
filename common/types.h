#pragma once

#include <jsonpp/json.hpp>
#include <iomanip>

namespace net
{
using json = nlohmann::json;
}

namespace git
{
struct tag
{
	std::string commit_id{};
	std::string id{};
};
} // namespace git

namespace nlohmann
{
template <>
struct adl_serializer<git::tag>
{
	static void to_json(json& j, const git::tag& opt);
	static void from_json(const json& j, git::tag& opt);
};
} // namespace nlohmann
