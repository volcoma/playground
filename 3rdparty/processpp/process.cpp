#include "process.h"

#ifdef _MSC_VER
#define popen _popen
#define pclose _pclose
#endif

namespace os
{

process::~process()
{
	close();
}

auto process::open(const std::string& cmd) -> bool
{
	file_ = popen(cmd.c_str(), "r");

	return is_open();
}

auto process::is_open() const -> bool
{
	return file_ != nullptr;
}

auto process::get_output() const -> std::string
{
	if(!is_open())
	{
		return {};
	}

	std::string output{};
	output.reserve(64);
	char byte{0};
	while(read(&byte, 1))
	{
		output += byte;
	}

	return output;
}

auto process::read(void* data, std::size_t size) const -> std::size_t
{
	return fread(data, 1, size, file_);
}

auto process::close() -> std::int32_t
{
	if(!is_open())
	{
		return -1;
	}
	std::int32_t exit_code = pclose(file_);
	file_ = nullptr;
	return exit_code;
}

auto syscall(const std::string& args) -> process
{
	process p;
	p.open(args);
	return p;
}

} // namespace os
