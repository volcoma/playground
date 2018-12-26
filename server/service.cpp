#include "service.h"
#include "git_utils.h"
#include <fstream>
namespace market
{

auto unique_filename(const std::string& tag, net::connection::id_t id) -> std::string
{
	return tag + "[" + std::to_string(id) + "]";
}

auto get_remote_json_file(const std::string& repo, const std::string& tag_id, const std::string& file,
						  const std::string& output_file) -> net::json
{
	net::json settings{};

	net::log() << "Download " + repo + "/" + tag_id + "/" + file + " > " + output_file + " ...";
	// download the file
	if(git::download_remote_file(repo, tag_id, file, output_file))
	{
		// load it
		std::ifstream file_stream{};
		file_stream.open(output_file);

		if(file_stream.is_open())
		{
			// deserialize it
			file_stream >> settings;
		}
	}

	// remove it
	std::remove(output_file.c_str());
	return settings;
}

auto get_remote_tags(const std::string& repo) -> std::vector<std::string>
{
	auto tags = git::fetch_remote_tags(repo);

	std::vector<std::string> result;
	result.reserve(tags.size());
	for(const auto& tag : tags)
	{
		result.emplace_back(tag.id);
	}
	return result;
}

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
		auto tags = get_remote_tags(repo);

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
			auto tag = in_msg.at("tag").get<std::string>();

			std::string output_file = unique_filename(tag, id) + ".json";
			auto settings = get_remote_json_file(repo, tag, file, output_file);

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
	auto cached_msg = cache_[id];

	pool_
		.schedule([id, in_msg = std::move(in_msg), cached_msg = std::move(cached_msg), repo = repo_,
				   file = file_]() {
			auto settings = in_msg.at("settings").get<net::json>();
			auto tag = in_msg.at("tag").get<std::string>();

			auto remote_settings = [&]() {
				try
				{
					return cached_msg.at("settings").get<net::json>();
				}
				catch(const std::exception&)
				{
					net::log() << "Expired cache.";
					std::string output_file = unique_filename(tag, id) + ".json";
					return get_remote_json_file(repo, tag, file, output_file);
				}
			}();

			// only removing is allowed
			auto ok = !settings.empty() && validate_settings(remote_settings, settings);

			net::log() << "Validation " << (ok ? "passed." : "failed.");

			if(ok)
			{
				std::string exported_name = "market[" + tag + "].json";
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
			msg["settings"] = std::move(remote_settings);
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
