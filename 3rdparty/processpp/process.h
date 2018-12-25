#pragma once
#include <cstdint>
#include <cstdio>
#include <string>

namespace os
{

class process
{
public:
	process() = default;
	~process();

	process(process&&) = default;
	process& operator=(process&&) = default;

	process(const process&) = delete;
	process& operator=(const process&) = delete;

	auto is_open() const -> bool;
	auto get_output() const -> std::string;
	auto open(const std::string& args) -> bool;
	auto close() -> std::int32_t;

private:
	auto read(void* _data, std::size_t _size) const -> std::size_t;

	FILE* file_{nullptr};
};

auto syscall(const std::string& args) -> process;

} // namespace os
