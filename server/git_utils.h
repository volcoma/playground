#pragma once
#include <common/types.h>

namespace git
{

auto get_remote_tags(const std::string& repo) -> std::vector<tag>;

auto download_remote_file(const std::string& repo, const std::string& tag_id, const std::string& file,
						  const std::string& output_file) -> bool;

auto load_remote_json_file(const std::string& repo, const std::string& tag_id, const std::string& file)
	-> net::json;
} // namespace git
