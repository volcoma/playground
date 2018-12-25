#include "console_client.h"
#include <common/string_utils.h>
namespace market
{

void console_client::init(net::connector_ptr connector)
{
	client::init(std::move(connector));

	input_thread_ = itc::make_thread("input_thread_");

	itc::invoke(input_thread_.get_id(), [this]() {
		while(!itc::this_thread::notified_for_exit())
		{
			std::string input;
			std::getline(std::cin, input);

			itc::invoke(itc::main_thread::get_id(), [this, input]() {
				if(input == "request_tags")
				{
					request_tags();
				}
				else if(input.find("request_settings") != std::string::npos)
				{
					auto input_line = string_utils::split(input, ' ', true);

					auto tag = input_line[1];
					const auto& tags = get_tags();
					auto it = std::find_if(std::begin(tags), std::end(tags),
										   [&tag](const auto& el) { return tag == el.id; });
					if(it != std::end(tags))
					{
						request_settings(*it);
					}
				}
				else if(input == "request_export")
				{
					request_export();
				}
				else if(input == "request_export_bad")
				{
					request_export_bad();
				}
				else if(input == "dump")
				{
					dump();
				}
			});
		}
	});
}

void console_client::shutdown()
{
	int* ptr = nullptr;
	*ptr = 42;

	if(input_thread_.joinable())
	{
		input_thread_.join();
	}
	client::shutdown();
}

} // namespace market
