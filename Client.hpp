#pragma once
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

#include "HTTP.hpp"
#include "Globals.hpp"
#include "TTSModel.hpp"

namespace FakeYou {
	class Client {
	private:
		std::time_t lastRequest;
		enum class APISlotState {
			Idle,
			SendingRequest,
			WaitingForResponse
		};
		APISlotState slot1;
		APISlotState slot2;

		std::string usernameOrEmail;
		std::string password;
	public:
		bool Login(std::string usernameOrEmail, std::string password)
		{
			this->usernameOrEmail = usernameOrEmail;
			this->password = password;

			std::string data = "{\"username_or_email\":\"" + usernameOrEmail + "\",\"password\":\"" + password + "\"}";
			std::map<std::string, std::string> headers = {
				{ "Content-Type", "application/json" },
				{ "Accept", "application/json" }
			};
			auto res = http::request(baseURL + "/login", "POST", headers, data);

			for (const auto& header : res.headers) {
				if (header.first == "set-cookie") {
					// Extract the "session" cookie from the "set-cookie" header
					std::string setCookie = header.second;
					size_t sessionPos = setCookie.find("session=");
					if (sessionPos != std::string::npos) {
						size_t endPos = setCookie.find_first_of(";", sessionPos);
						if (endPos == std::string::npos) {
							authCookie = setCookie.substr(sessionPos + 8); // 8 is the length of "session="
						}
						else {
							authCookie = setCookie.substr(sessionPos + 8, endPos - sessionPos - 8);
						}
					}
					break; // If "session" cookie found, no need to continue the loop
				}
			}

			nlohmann::json body = nlohmann::json::parse(res.body);
			if (body["success"])
				return true;
			else
			{
				authCookie = "";
				return false;
			}
		}
		bool Logout()
		{
			this->usernameOrEmail = "";
			this->password = "";

			auto res = http::request(baseURL + "/logout", "POST");

			authCookie = "";

			nlohmann::json body = nlohmann::json::parse(res.body);
			return body["success"];
		}

		TTSModel* GetModelByToken(std::string token)
		{
			auto models = GetModels();
			for (int i = 0; i < models.size(); i++) {
				if (models[i]->modelToken == token)
					return models[i];
			}

			return nullptr;
		}
		TTSModel* GetModelByTitle(std::string title)
		{
			auto models = GetModels();
			for (int i = 0; i < models.size(); i++) {
				if (models[i]->title == title)
					return models[i];
			}

			return nullptr;
		}
		std::vector<TTSModel*> GetModels()
		{
			std::vector<TTSModel*> models;

			std::map<std::string, std::string> headers = {
				{ "Content-Type", "application/json" },
				{ "Accept", "application/json" },
				{ "Credentials", "include" },
				{ "Cookies", "session=" + authCookie },
			};
			auto res = http::request(baseURL + "/tts/list", "GET", headers);
			nlohmann::json body = nlohmann::json::parse(res.body);

			try {
				for (const auto& modelData : body["models"]) {
					TTSModel* model = new TTSModel;
					model->modelToken = modelData["model_token"];
					model->ttsModelType = modelData["tts_model_type"];
					model->creatorUserToken = modelData["creator_user_token"];
					model->creatorUsername = modelData["creator_username"];
					model->creatorDisplayName = modelData["creator_display_name"];
					model->creatorGravatarHash = modelData["creator_gravatar_hash"];
					model->title = modelData["title"];
					model->ietfLanguageTag = modelData["ietf_language_tag"];
					model->ietfPrimaryLanguageSubtag = modelData["ietf_primary_language_subtag"];
					model->isFrontPageFeatured = modelData["is_front_page_featured"];
					model->isTwitchFeatured = modelData["is_twitch_featured"];
					model->maybeSuggestedUniqueBotCommand = modelData["maybe_suggested_unique_bot_command"];
					model->creatorSetVisibility = modelData["creator_set_visibility"];
					model->userRatings.positiveCount = modelData["user_ratings"]["positive_count"];
					model->userRatings.negativeCount = modelData["user_ratings"]["negative_count"];
					model->userRatings.totalCount = modelData["user_ratings"]["total_count"];

					// Decode the category_tokens array
					for (const auto& category : modelData["category_tokens"]) {
						model->categoryTokens.push_back(category);
					}

					model->createdAt = modelData["created_at"];
					model->updatedAt = modelData["updated_at"];

					models.push_back(model);
				}
			}
			catch (const std::exception& e) {
				std::cerr << "Error decoding JSON: " << e.what() << std::endl;
			}

			return models;
		}
	};
}