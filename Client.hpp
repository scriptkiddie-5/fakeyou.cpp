#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <nlohmann/json.hpp>
#include <deque>
#include <chrono>
#include <mutex>

#include "HTTP.hpp"
#include "Globals.hpp"
#include "Profile.hpp"
#include "Leaderboard.hpp"
#include "TTS.hpp"
#include "VTV.hpp"

namespace FakeYou {
	class Client {
	private:
		std::string username;
		std::string password;
	public:
		bool Login(std::string username, std::string password)
		{
			std::cout << "Logging in as " + username << std::endl;
			enforceRateLimit();

			this->username = username;
			this->password = password;

			std::string data = "{\"username_or_email\":\"" + username + "\",\"password\":\"" + password + "\"}";
			std::map<std::string, std::string> headers = {
				{ "Content-Type", "application/json" },
				{ "Accept", "application/json" }
			};
			http::Response res = http::request(baseURL + "/login", "POST", headers, data);

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

			try {
				nlohmann::json body = nlohmann::json::parse(res.body);
				if (body["success"])
				{
					std::cout << "Logged in as " + username << std::endl;
					return true;
				}
			}
			catch (const std::exception e) {  }
			authCookie = "";
			std::cout << "Unable to log in as " + username << std::endl;
			return false;
		}

		bool Logout()
		{
			enforceRateLimit();

			this->username = "";
			this->password = "";

			http::Response res = http::request(baseURL + "/logout", "POST");

			authCookie = "";

			nlohmann::json body = nlohmann::json::parse(res.body);
			return body["success"];
		}

		Leaderboard* FetchLeaderboard()
		{
			enforceRateLimit();

			std::map<std::string, std::string> headers = {
				{ "Content-Type", "application/json" },
				{ "Accept", "application/json" }
			};

			http::Response res = http::request(baseURL + "/leaderboard", "GET", headers);

			try {
				nlohmann::json json = nlohmann::json::parse(res.body);
				if (!json["success"]) {
					return nullptr;
				}

				Leaderboard* leaderboard = new Leaderboard;

				for (const auto& entryJson : json["tts_leaderboard"]) {
					LeaderboardEntry* entry = new LeaderboardEntry;
					entry->creatorUserToken = entryJson["creator_user_token"];
					entry->username = entryJson["username"];
					entry->displayName = entryJson["display_name"];
					entry->gravatarHash = entryJson["gravatar_hash"];
					entry->defaultAvatarIndex = entryJson["default_avatar_index"];
					entry->defaultAvatarColorIndex = entryJson["default_avatar_color_index"];
					entry->uploadedCount = entryJson["uploaded_count"];
					leaderboard->ttsLeaderboard.push_back(entry);
				}

				for (const auto& entryJson : json["w2l_leaderboard"]) {
					LeaderboardEntry* entry = new LeaderboardEntry;
					entry->creatorUserToken = entryJson["creator_user_token"];
					entry->username = entryJson["username"];
					entry->displayName = entryJson["display_name"];
					entry->gravatarHash = entryJson["gravatar_hash"];
					entry->defaultAvatarIndex = entryJson["default_avatar_index"];
					entry->defaultAvatarColorIndex = entryJson["default_avatar_color_index"];
					entry->uploadedCount = entryJson["uploaded_count"];
					leaderboard->w2lLeaderboard.push_back(entry);
				}

				return leaderboard;
			}
			catch (const std::exception) {  }

			return nullptr;
		}

		Profile* FetchProfile(std::string username)
		{
			enforceRateLimit();

			std::map<std::string, std::string> headers = {
				{ "Content-Type", "application/json" },
				{ "Accept", "application/json" }
			};

			if (authCookie.size()) {
				headers.insert({
					{ "Credentials", "include" },
					{ "Cookie", std::string("session=" + authCookie) }
				});
			}

			http::Response res = http::request(baseURL + "/user/" + username + "/profile", "GET", headers);

			try {
				nlohmann::json json = nlohmann::json::parse(res.body);
				Profile* profile = new Profile;

				// Assign values from JSON to the Profile struct
				profile->userToken = json["user"]["user_token"];
				profile->username = json["user"]["username"];
				profile->displayName = json["user"]["display_name"];
				profile->emailGravatarHash = json["user"]["email_gravatar_hash"];
				profile->defaultAvatarIndex = json["user"]["default_avatar_index"];
				profile->defaultAvatarColorIndex = json["user"]["default_avatar_color_index"];
				profile->profileMarkdown = json["user"]["profile_markdown"];
				profile->profileRenderedHtml = json["user"]["profile_rendered_html"];
				profile->userRoleSlug = json["user"]["user_role_slug"];
				profile->disableGravatar = json["user"]["disable_gravatar"];
				profile->preferredTTSResultVisibility = json["user"]["preferred_tts_result_visibility"];
				profile->preferredW2LResultVisibility = json["user"]["preferred_w2l_result_visibility"];
				profile->discordUsername = json["user"]["discord_username"].is_null() ? "" : json["user"]["discord_username"];
				profile->twitchUsername = json["user"]["twitch_username"].is_null() ? "" : json["user"]["twitch_username"];
				profile->twitterUsername = json["user"]["twitter_username"].is_null() ? "" : json["user"]["twitter_username"];
				profile->patreonUsername = json["user"]["patreon_username"].is_null() ? "" : json["user"]["patreon_username"];
				profile->githubUsername = json["user"]["github_username"].is_null() ? "" : json["user"]["github_username"];
				profile->cashappUsername = json["user"]["cashapp_username"].is_null() ? "" : json["user"]["cashapp_username"];
				profile->websiteUrl = json["user"]["website_url"].is_null() ? "" : json["user"]["website_url"];

				// Parse and assign badge data
				for (const auto& badgeJson : json["user"]["badges"]) {
					Profile::Badge badge;
					badge.slug = badgeJson["slug"];
					badge.title = badgeJson["title"];
					badge.description = badgeJson["description"];
					badge.imageUrl = badgeJson["image_url"];
					badge.grantedAt = badgeJson["granted_at"];
					profile->badges.push_back(badge);
				}

				profile->createdAt = json["user"]["created_at"].is_null() ? "" : json["user"]["created_at"];
				profile->maybeModeratorFields = json["user"]["maybe_moderator_fields"].is_null() ? "" : json["user"]["maybe_moderator_fields"];

				return profile;
			}
			catch (const std::exception e) {  }

			return nullptr;
		}

		Profile* FetchMyProfile()
		{
			// already rate limit enforced
			return FetchProfile(this->username);
		}

		TTS* tts;
		VTV* vtv;
	};
}