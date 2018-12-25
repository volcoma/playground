#pragma once
#include <common/net.h>

namespace market
{

class client
{
public:
	void init(net::connector_ptr connector);
	void shutdown();

	void request_tags();
	void request_settings(const git::tag& tag);
	void request_export();
    void request_export_bad();

	void on_tags_recieved(net::json in_msg);
	void on_settings_recieved(net::json in_msg);
	void on_export_recieved(net::json in_msg);

    auto get_tags() const -> const std::vector<git::tag>&;
    auto get_settings() const -> const net::json&;

    void dump();

private:
	std::vector<git::tag> tags_;
	net::json settings_;
	net::json export_;

	net::connection::id_t connection_id_{};
};

} // namespace market
