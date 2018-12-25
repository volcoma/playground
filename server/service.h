#pragma once
#include <common/net.h>
#include <itc/thread_pool.h>

namespace market
{

class service
{
public:
    service(std::string repo, std::string file);
	void init(net::connector_ptr connector);
	void shutdown();

	void on_tags_requested(net::connection::id_t id, net::json in_msg);
	void on_settings_requested(net::connection::id_t id, net::json in_msg);
    void on_export_requested(net::connection::id_t id, net::json in_msg);

private:
	itc::thread_pool pool_;

	std::string repo_;
	std::string file_;

	// this should only be touched from the
	// creation thread
	std::map<net::connection::id_t, net::json> cache_;
};

} // namespace market
