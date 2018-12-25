#include "types.h"

namespace nlohmann
{

void adl_serializer<git::tag>::to_json(nlohmann::json &j, const git::tag &opt)
{
    j["commit_id"] = opt.commit_id;
    j["id"] = opt.id;
}


void adl_serializer<git::tag>::from_json(const json &j, git::tag &opt)
{
    opt.commit_id = j.at("commit_id");
    opt.id = j.at("id");
}


}
