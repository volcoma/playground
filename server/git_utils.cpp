#include "git_utils.h"
#include <common/string_utils.h>
#include <processpp/process.h>

#include <algorithm>
#include <fstream>
namespace git
{

auto fetch_remote_tags(const std::string& repo) -> std::vector<tag>
{
	std::vector<tag> tags{};

	std::string cmd{};
	cmd.append("git ls-remote");
	cmd.append(" ");
	cmd.append("--tags");
	cmd.append(" ");
	cmd.append(repo);

	auto output = os::syscall(cmd).get_output();
	auto lines = string_utils::tokenize(output, "\n");

	for(const auto& line : lines)
	{
		auto tag_data = string_utils::tokenize(line, "\t");

		if(tag_data.size() != 2)
		{
			continue;
		}

		tags.emplace_back();
		auto& tag = tags.back();

		tag.commit_id = tag_data.at(0);
		tag.full_id = tag_data.at(1);

		const std::string word{"/tags/"};
		auto pos = tag.full_id.find(word);
		if(pos != std::string::npos)
		{
			tag.id = tag.full_id.substr(pos + word.size());
		}
	}

	return tags;
}

auto download_remote_file(const std::string& repo, const std::string& tag_id, const std::string& file,
						  const std::string& output_file) -> bool
{

	//	std::string cmd{};
	//	cmd.append("curl -LJ " + repo + "/raw/");
	//	cmd.append(tag_id);
	//	cmd.append("/");
	//	cmd.append(file);
	//	cmd.append(" > ");
	//	cmd.append(output_file);

	//	std::string cmd{};
	//	cmd.append("git archive");
	//	cmd.append(" ");
	//	cmd.append("--remote=");
	//	cmd.append(repo);
	//	cmd.append(" ");
	//	cmd.append(tag_id);
	//	cmd.append(" ");
	//	cmd.append(file);
	//	cmd.append(" | tar xf -");

	const auto& tmp_repo = tag_id;

	std::string remove_cmd;
#ifdef _WIN32
	remove_cmd.append("if exist " + tmp_repo + " (rd /s /q " + tmp_repo + ")");
#elif
	remove_cmd.append("rm -rf " + tmp_repo);
#endif
	{
		os::syscall(remove_cmd);
	}

	// --branch also accepts tags
	std::string cmd{};
	cmd.append("git clone --depth 1 --no-checkout --branch " + tag_id);
	cmd.append(" " + repo + " " + tmp_repo);
	cmd.append(" && ");
	cmd.append("cd " + tmp_repo);
	cmd.append(" && ");
	cmd.append("git show " + tag_id + ":" + file + " > ../" + output_file);
	cmd.append(" && ");
	cmd.append("cd ..");
	cmd.append(" && ");
	cmd.append(remove_cmd);

	return os::syscall(cmd).close() == 0;
}

} // namespace git
