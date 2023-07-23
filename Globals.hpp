#pragma once

namespace FakeYou {
	extern void enforceRateLimit();
	extern std::deque<std::chrono::time_point<std::chrono::steady_clock>> requestQueue;
	extern std::mutex queueMutex;

	extern std::string baseURL;
	extern std::string authCookie;
	extern std::string GenerateUUIDV4();
}