#include "client.h"

namespace market
{

void client::init(net::connector_ptr connector)
{
	try
	{
		auto net = net::json_net();
		net->add_connector(std::move(connector),
						   [this](net::connection::id_t id) {
							   net::log() << "Connected.";

							   connection_id_ = id;
						   },
						   [this](net::connection::id_t id, const net::error_code& ec) {
							   connection_id_ = 0;
							   net::log() << "Disconnected. Reason : " << ec.message();
						   },
						   [this](net::connection::id_t id, net::json msg) {
							   try
							   {
								   net::log() << "---------------------------------------------";
								   net::log() << "Recieved msg :\n" << std::setw(4) << msg;
								   auto msg_id = msg.at("id").get<std::string>();

								   if(msg_id == "tags")
								   {
									   on_tags_recieved(std::move(msg));
								   }
								   else if(msg_id == "settings")
								   {
									   on_settings_recieved(std::move(msg));
								   }
								   else if(msg_id == "export")
								   {
									   on_export_recieved(std::move(msg));
								   }
							   }
							   catch(const std::exception& e)
							   {
								   net::log() << e.what();
							   }
						   });
	}
	catch(std::exception& e)
	{
		net::log() << "Exception: " << e.what();
	}
}

void client::shutdown()
{
}

void client::request_tags()
{
	tags_ = {};
	settings_ = {};

	net::json msg;
	msg["id"] = "request_tags";

	auto net = net::json_net();
	net->send_msg(connection_id_, std::move(msg));

	net::log() << "Requested tags. Please wait...";
}

void client::request_settings(const std::string& tag)
{
	settings_ = {};

	net::json msg;
	msg["id"] = "request_settings";
	msg["tag"] = tag;

	auto net = net::json_net();
	net->send_msg(connection_id_, std::move(msg));

	net::log() << "Requested settings. Please wait...";
}

void client::request_export()
{
	try
	{
		auto settings = settings_.at("settings").get<net::json>();
		auto tag = settings_.at("tag").get<std::string>();

		net::json msg;
		msg["id"] = "request_export";
		msg["settings"] = std::move(settings);
		msg["tag"] = std::move(tag);

		auto net = net::json_net();
		net->send_msg(connection_id_, std::move(msg));

		net::log() << "Requested export. Please wait...";
	}
	catch(const std::exception& e)
	{
		net::log() << e.what();
	}
}

void client::request_export_bad()
{
	try
	{
		auto settings = settings_.at("settings").get<net::json>();
		auto tag = settings_.at("tag").get<std::string>();
		settings["some_add"] = 12;

		net::json msg;
		msg["id"] = "request_export";
		msg["settings"] = std::move(settings);
		msg["tag"] = std::move(tag);

		auto net = net::json_net();
		net->send_msg(connection_id_, std::move(msg));

		net::log() << "Requested export. Please wait...";
	}
	catch(const std::exception& e)
	{
		net::log() << e.what();
	}
}

void client::on_tags_recieved(net::json in_msg)
{
	tags_ = in_msg.at("tags").get<std::vector<std::string>>();
}

void client::on_settings_recieved(net::json in_msg)
{
	settings_ = std::move(in_msg);
}

void client::on_export_recieved(net::json in_msg)
{
	export_ = std::move(in_msg);
}

auto client::get_tags() const -> const std::vector<std::string>&
{
	return tags_;
}

auto client::get_settings() const -> const net::json&
{
	return settings_;
}

void client::dump()
{
	net::log() << std::setw(4) << settings_;
}

} // namespace market
