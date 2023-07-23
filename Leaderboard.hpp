#pragma once

namespace FakeYou {
    struct LeaderboardEntry {
        std::string creatorUserToken;
        std::string username;
        std::string displayName;
        std::string gravatarHash;
        int defaultAvatarIndex;
        int defaultAvatarColorIndex;
        int uploadedCount;
    };

    struct Leaderboard {
        std::vector<LeaderboardEntry*> ttsLeaderboard;
        std::vector<LeaderboardEntry*> w2lLeaderboard;
    };
}