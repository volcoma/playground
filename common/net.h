#pragma once

#include <asiopp/service.h>
#include <builderpp/msg_builder.h>
#include <messengerpp/messenger.h>

#include <jsonpp/json.hpp>
#include <iomanip>

namespace net
{
using json = nlohmann::json;

template <>
struct serializer<json, json, json>
{
	static auto to_buffer(const json& msg) -> byte_buffer
	{
		return json::to_cbor(msg);
	}
	static auto from_buffer(byte_buffer&& buffer) -> json
	{
		return json::from_cbor(buffer);
	}
};

inline auto json_net()
{
	return get_messenger<json, json, json>();
}

} // namespace net
