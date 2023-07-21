#pragma once
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <random>
#include <iomanip>
#include <sstream>

#include "Client.hpp"

namespace FakeYou {
	class TTSResult {
	public:
		TTSResult(std::string url) : url(url) {}

		std::string url;
	};
}