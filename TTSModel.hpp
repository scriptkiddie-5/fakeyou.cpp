#pragma once
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <random>
#include <iomanip>
#include <sstream>

#include "Client.hpp"
#include "TTSResult.hpp"

namespace FakeYou {
	class TTSModel {
	public:
        TTSResult* Infer(std::string text) {
            // Step 1: Send inference request
            std::map<std::string, std::string> headers = {
                { "Content-Type", "application/json" },
                { "Accept", "application/json" },
                { "Credentials", "include" },
                { "Cookie", std::string("session=" + authCookie) },
            };

            std::string data = "{\"tts_model_token\":\"" + this->modelToken + "\",\"uuid_idempotency_token\":\"" + GenerateUUIDV4() + "\",\"inference_text\":\"" + text + "\"}";
            http::HttpResponse response = http::request(baseURL + "/tts/inference", "POST", headers, data);

            try {
                nlohmann::json body = nlohmann::json::parse(response.body);

                // Step 2: Check if the inference request was successful
                bool success = body["success"];
                if (!success) {
                    // Handle the error if necessary
                    return nullptr; // Return an empty string to indicate failure
                }

                // Step 3: Keep polling for job status
                while (true) {
                    std::string inferenceJobToken = body["inference_job_token"];
                    http::HttpResponse jobResponse = http::request(baseURL + "/tts/job/" + inferenceJobToken, "GET", headers);
                    nlohmann::json jobBody = nlohmann::json::parse(jobResponse.body);
                    // Check if the "error_message" field exists in the JSON object
                    if (jobBody.contains("error_message") && jobBody["error_message"].is_string()) {
                        std::string error_message = jobBody["error_message"].get<std::string>();
                        if (!error_message.empty()) {
                            // Handle the error message here if needed
                            // ...
                            return nullptr;
                        }
                    }
                    bool jobSuccess = jobBody["success"];
                    if (jobSuccess && jobBody["state"]["status"] == "complete_success") {
                        // Step 4: Return the URL for the generated audio if the job is successful
                        std::string audioPath = jobBody["state"]["maybe_public_bucket_wav_audio_path"];
                        if (!audioPath.empty()) {
                            TTSResult* result = new TTSResult("https://storage.googleapis.com/vocodes-public" + audioPath);
                            return result;
                        }
                    }

                    // Sleep for a second before polling again
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }
            catch (const nlohmann::json::parse_error& e)
            {
                // Handle the case when JSON parsing fails
                std::cerr << "Error parsing JSON response: " << e.what() << std::endl;
            }

            // This return statement is here just to suppress compiler warnings about not all control paths returning a value
            return nullptr;
        }

        std::string modelToken;
        std::string ttsModelType;
        std::string creatorUserToken;
        std::string creatorUsername;
        std::string creatorDisplayName;
        std::string creatorGravatarHash;
        std::string title;
        std::string ietfLanguageTag;
        std::string ietfPrimaryLanguageSubtag;
        bool isFrontPageFeatured;
        bool isTwitchFeatured;
        nlohmann::json maybeSuggestedUniqueBotCommand; // We'll use json to represent null values
        std::string creatorSetVisibility;
        struct UserRatings {
            int positiveCount;
            int negativeCount;
            int totalCount;
        } userRatings;
        std::vector<std::string> categoryTokens;
        std::string createdAt;
        std::string updatedAt;
    private:
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
	};
}