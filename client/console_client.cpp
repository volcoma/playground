#include "console_client.h"
#include <common/string_utils.h>
namespace market
{
bool contains(const std::string& input, const std::string& substr)
{
    return input.find(substr) != std::string::npos;
}

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
				if(contains(input, "request_tags"))
				{
					request_tags();
				}
				else if(contains(input, "request_settings"))
				{
                    // tokenize by spaces or tabs
					auto input_line = string_utils::tokenize(input, " \t");
					if(input_line.size() > 1)
					{
                        auto tag = input_line.at(1);
                        const auto& tags = get_tags();
                        auto it = std::find(std::begin(tags), std::end(tags), tag);
                        if(it != std::end(tags))
                        {
                            request_settings(*it);
                        }
					}
				}
				else if(contains(input, "request_export"))
				{
                    // tokenize by spaces or tabs
					auto input_line = string_utils::tokenize(input, " \t");
					if(input_line.size() > 1)
					{
                        auto name = input_line.at(1);
                        request_export(name);
					}
				}
				else if(contains(input, "dump"))
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
