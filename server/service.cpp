#include "service.h"
#include "git_utils.h"
#include <fstream>
namespace market
{

bool validate_settings(const net::json& original_settings, const net::json& settings)
{
	auto diff = net::json::diff(original_settings, settings);

	for(const auto& op : diff)
	{
		std::string op_id = op.at("op");
		if(op_id != "remove")
		{
			return false;
		}
	}
	return true;
}

service::service(std::string repo, std::string file)
	: repo_(std::move(repo))
	, file_(std::move(file))
{
}

void service::init(net::connector_ptr connector)
{
	auto net = net::json_net();
	net->add_connector(std::move(connector),
					   [](net::connection::id_t id) {
						   net::log() << "Connected client " << id;
						   (void)id;
					   },
					   [this](net::connection::id_t id, const net::error_code& ec) {
						   net::log() << "Disconnected client " << id << ". Reason : " << ec.message();

						   cache_.erase(id);
					   },
					   [this](net::connection::id_t id, net::json msg) {
						   try
						   {
							   net::log() << "---------------------------------------------";
							   net::log() << "Recieved msg from client " << id << " :\n"
										  << std::setw(4) << msg;

							   auto msg_id = msg.at("id").get<std::string>();

							   if(msg_id == "request_tags")
							   {
								   on_tags_requested(id, std::move(msg));
							   }
							   else if(msg_id == "request_settings")
							   {
								   on_settings_requested(id, std::move(msg));
							   }
							   else if(msg_id == "request_export")
							   {
								   on_export_requested(id, std::move(msg));
							   }
						   }
						   catch(const std::exception& e)
						   {
							   net::log() << e.what();
						   }
					   });
}

void service::shutdown()
{
	pool_.stop_all();
	pool_.wait_all();
}

void service::on_tags_requested(net::connection::id_t id, net::json in_msg)
{
	(void)in_msg;

	pool_.schedule([id, repo = repo_]() {
		auto tags = git::get_remote_tags(repo);

		net::json msg;
		msg["id"] = "tags";
		msg["tags"] = std::move(tags);

		auto net = net::json_net();
		net->send_msg(id, std::move(msg));
	});
}

void service::on_settings_requested(net::connection::id_t id, net::json in_msg)
{
	pool_
		.schedule([id, in_msg = std::move(in_msg), repo = repo_, file = file_]() mutable {
			auto tag = in_msg.at("tag").get<git::tag>();
			auto settings = git::load_remote_json_file(repo, tag.id, file);

			net::json msg;
			msg["id"] = "settings";
			msg["settings"] = std::move(settings);
			msg["tag"] = std::move(tag);

			auto net = net::json_net();
			net->send_msg(id, net::json(msg));

			return msg;
		})
		.then(itc::this_thread::get_id(), [this, id](auto f) {
			try
			{
				auto msg = f.get();
				cache_[id] = std::move(msg);
				net::log() << "Updating client " << id << " cache.";
			}
			catch(const std::exception& e)
			{
				net::log() << e.what();
			}
		});
}

void service::on_export_requested(net::connection::id_t id, net::json in_msg)
{
	auto og_msg = cache_[id];

	pool_
		.schedule([id, in_msg = std::move(in_msg), og_msg = std::move(og_msg), repo = repo_, file = file_]() {
			auto settings = in_msg.at("settings").get<net::json>();
			auto tag = in_msg.at("tag").get<git::tag>();

			auto og_settings = [&]() {
				try
				{
					return og_msg.at("settings").get<net::json>();
				}
				catch(const std::exception&)
				{
					net::log() << "Expired cache.";
					return git::load_remote_json_file(repo, tag.id, file);
				}
			}();

			// only removing is allowed
			auto ok = !settings.empty() && validate_settings(og_settings, settings);

			net::log() << "Validation " << (ok ? "passed." : "failed.");

			if(ok)
			{
				std::string exported_name = "market[" + tag.id + "].json";
				std::ofstream o(exported_name);
				o << std::setw(4) << settings << std::endl;
			}
			else
			{
				net::log() << "Only removal is allowed.";
			}
			{
				net::json msg;
				msg["id"] = "export";
				msg["export_result"] = ok;

				auto net = net::json_net();
				net->send_msg(id, std::move(msg));
			}

			// update the cache
			net::json msg;
			msg["id"] = "settings";
			msg["settings"] = std::move(og_settings);
			msg["tag"] = std::move(tag);

			return msg;
		})
		.then(itc::this_thread::get_id(), [this, id](auto f) {
			try
			{
				auto msg = f.get();
				cache_[id] = std::move(msg);
				net::log() << "Updating client " << id << " cache.";
			}
			catch(const std::exception& e)
			{
				net::log() << e.what();
			}
		});
}
} // namespace market
