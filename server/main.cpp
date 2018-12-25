#include "service.h"

int main(int argc, char* argv[])
{

	try
	{
		auto info_logger = [](const std::string& msg) { std::cout << msg << std::endl; };

		itc::init_data init;
		init.log_info = info_logger;
		init.log_error = info_logger;
		itc::init(init);
		net::set_logger(info_logger);
		net::init_services();

		std::string repo = "https://github.com/volcoma/playground";
		std::string file = "data/template.json";
		uint16_t port = 11111;

		market::service service(std::move(repo), std::move(file));

		auto connector = net::create_tcp_server(port);
		if(!connector)
		{
			return -1;
		}
		connector->create_builder = net::msg_builder::get_creator<net::single_buffer_builder>();

		service.init(std::move(connector));

		while(!itc::this_thread::notified_for_exit())
		{
			itc::this_thread::wait();
		}

		service.shutdown();

		net::deinit_messengers();
		net::deinit_services();
		return itc::shutdown();
	}
	catch(std::exception& e)
	{
		net::log() << "Exception: " << e.what();
	}

	return -1;
}
