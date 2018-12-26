#pragma once
#include <string>
#include <vector>

namespace git
{
struct tag
{
	std::string commit_id{};
	std::string full_id{};
	std::string id{};
};
auto fetch_remote_tags(const std::string& repo) -> std::vector<tag>;

auto download_remote_file(const std::string& repo, const std::string& tag_id, const std::string& file,
						  const std::string& output_file) -> bool;

} // namespace git
