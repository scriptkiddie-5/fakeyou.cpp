#pragma once
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <random>
#include <iomanip>
#include <sstream>

#include "Client.hpp"

namespace FakeYou {
	class VTVModel {
    public:
        std::string token;
        std::string modelType;
        std::string title;
        struct Creator {
            std::string userToken;
            std::string username;
            std::string displayName;
            std::string gravatarHash;
        } creator;
        std::string creatorSetVisibility;
        std::string ietfLanguageTag;
        std::string ietfPrimaryLanguageSubtag;
        bool isFrontPageFeatured;
        std::string createdAt;
        std::string updatedAt;
	};
    class VTV {
    public:
		VTVModel* GetModelByToken(std::string token)
		{
			auto models = GetModels();
			for (int i = 0; i < models.size(); i++) {
				if (models[i]->token == token)
					return models[i];
			}

			return nullptr;
		}
		VTVModel* GetModelByTitle(std::string title)
		{
			auto models = GetModels();
			for (int i = 0; i < models.size(); i++) {
				if (models[i]->title == title)
					return models[i];
			}

			return nullptr;
		}
		std::vector<VTVModel*> GetModels()
		{
			std::vector<VTVModel*> models;

			std::map<std::string, std::string> headers = {
                { "Content-Type", "application/json" },
                { "Accept", "application/json" }
            };

            if (authCookie.size())
                headers.insert({
                    { "Credentials", "include" },
                    { "Cookie", std::string("session=" + authCookie) }
                });

			http::HttpResponse res = http::request(baseURL + "/v1/voice_conversion/model_list", "GET", headers);
			nlohmann::json body = nlohmann::json::parse(res.body);

			try {
				for (const auto& modelData : body["models"]) {
					VTVModel* model = new VTVModel;

					model->token = modelData["token"];
					model->modelType = modelData["model_type"];
					model->title = modelData["title"];

					model->creator.userToken = modelData["creator"]["user_token"];
					model->creator.username = modelData["creator"]["username"];
					model->creator.displayName = modelData["creator"]["display_name"];
					model->creator.gravatarHash = modelData["creator"]["gravatar_hash"];

					model->creatorSetVisibility = modelData["creator_set_visibility"];
					model->ietfLanguageTag = modelData["ietf_language_tag"];
					model->ietfPrimaryLanguageSubtag = modelData["ietf_primary_language_subtag"];
					model->isFrontPageFeatured = modelData["is_front_page_featured"];
					model->createdAt = modelData["created_at"];
					model->updatedAt = modelData["updated_at"];

					models.push_back(model);
				}
			}
			catch (const std::exception& e) {  }

			return models;
		}
    };
}