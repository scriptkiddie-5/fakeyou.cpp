#include <string>
#include <random>
#include <sstream>
#include <thread>
#include <deque>
#include <chrono>
#include <mutex>

#include "Globals.hpp"

// define globals
namespace FakeYou {
	// Helper function to enforce rate limits
	void enforceRateLimit() {
		std::unique_lock<std::mutex> lock(queueMutex);
		
		if (requestQueue.size() >= 2) {
			auto timeSinceOldestRequest = std::chrono::steady_clock::now() - requestQueue.front();
			if (timeSinceOldestRequest < std::chrono::seconds(1)) {
				auto sleepDuration = std::chrono::seconds(1) - timeSinceOldestRequest;
				std::this_thread::sleep_for(sleepDuration);
			}
			requestQueue.pop_front();
		}

		requestQueue.push_back(std::chrono::steady_clock::now());
	}

	// Rate limiting variables
	std::deque<std::chrono::time_point<std::chrono::steady_clock>> requestQueue;
	std::mutex queueMutex;

	std::string baseURL = "https://api.fakeyou.com";
	std::string authCookie = "";
	std::string GenerateUUIDV4() {
		// Generate a random number using a random device
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<uint32_t> dist(0, 15);

		// Create a function to convert an integer to a hexadecimal character
		auto to_hex = [](uint32_t value) -> char {
			return "0123456789abcdef"[value];
		};

		std::stringstream ss;
		for (int i = 0; i < 32; ++i) {
			// Generate four random numbers to represent the 32 hexadecimal digits
			uint32_t random_value = dist(gen);
			ss << to_hex(random_value);
		}

		// Insert dashes to form the UUID v4 format
		std::string uuid = ss.str();
		uuid.insert(8, "-");
		uuid.insert(13, "-");
		uuid.insert(18, "-");
		uuid.insert(23, "-");

		return uuid;
	}
}