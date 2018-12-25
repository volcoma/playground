#pragma once
#include "client.h"

namespace market
{

class console_client : public client
{
public:
	void init(net::connector_ptr connector);
	void shutdown();

private:
	itc::thread input_thread_;
};

} // namespace market
