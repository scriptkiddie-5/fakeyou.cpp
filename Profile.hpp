#pragma once
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <random>
#include <iomanip>
#include <sstream>
#include <fstream>

#include "Client.hpp"

namespace FakeYou {
	struct Profile {
		std::string userToken;
		std::string username;
		std::string displayName;
		std::string emailGravatarHash;
		int defaultAvatarIndex;
		int defaultAvatarColorIndex;
		std::string profileMarkdown;
		std::string profileRenderedHtml;
		std::string userRoleSlug;
		bool disableGravatar;
		std::string preferredTTSResultVisibility;
		std::string preferredW2LResultVisibility;
		std::string discordUsername;
		std::string twitchUsername;
		std::string twitterUsername;
		std::string patreonUsername;
		std::string githubUsername;
		std::string cashappUsername;
		std::string websiteUrl;
		struct Badge {
			std::string slug;
			std::string title;
			std::string description;
			std::string imageUrl;
			std::string grantedAt;
		};
		std::vector<Badge> badges;
		std::string createdAt;
		std::string maybeModeratorFields;
	};
}