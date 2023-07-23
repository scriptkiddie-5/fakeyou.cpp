#pragma once
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <random>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <thread>

#include "Globals.hpp"
#include "HTTP.hpp"

namespace FakeYou {
    class TTSResult {
    public:
        TTSResult(std::string url) : url(url) {}

        bool Download(std::string filepath, std::string filename)
        {
            // Make an HTTP GET request to download the WAV file
            http::Response response = http::request(this->url, "GET");

            std::string path = filepath + "/" + filename + ".wav";

            // Save the downloaded WAV data to a local file
            std::ofstream file(path, std::ios::binary);
            if (file) {
                file.write(response.body.c_str(), response.body.size());
                file.close();
                return true;
            }
            else {
                return false;
            }
        }

        std::string url;
    };
    class TTSModel {
    public:
        // classes
        enum class Rating : int {
            Positive = 0,
            Neutral = 1,
            Negative = 2,
            Unknown = 3
        };
        enum class Visibility : int {
            Public = 0,
            Hidden = 1,
            Private = 2,
            Unknown = 3
        };

        TTSResult* Inference(std::string text) {
            enforceRateLimit();

            // Step 1: Send inference request
            std::map<std::string, std::string> headers = {
                { "Content-Type", "application/json" },
                { "Accept", "application/json" }
            };

            if (authCookie.size())
                headers.insert({
                    { "Credentials", "include" },
                    { "Cookie", std::string("session=" + authCookie) }
                });

            std::string data = "{\"tts_model_token\":\"" + this->modelToken + "\",\"uuid_idempotency_token\":\"" + GenerateUUIDV4() + "\",\"inference_text\":\"" + text + "\"}";
            http::Response response = http::request(baseURL + "/tts/inference", "POST", headers, data);

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
                    http::Response jobResponse = http::request(baseURL + "/tts/job/" + inferenceJobToken, "GET", headers);
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
            catch (const std::exception e)
            {
                return nullptr;
            }

            // This return statement is here just to suppress compiler warnings about not all control paths returning a value
            return nullptr;
        }
        Rating GetMyRating()
        {
            if (!authCookie.size())
                return Rating::Unknown;

            enforceRateLimit();

            std::map<std::string, std::string> headers = {
                { "Content-Type", "application/json" },
                { "Accept", "application/json" }
            };

            if (authCookie.size())
                headers.insert({
                    { "Credentials", "include" },
                    { "Cookie", std::string("session=" + authCookie) }
                });

            try
            {
                http::Response response = http::request(baseURL + "/v1/user_rating/view/tts_model/" + this->modelToken, "GET", headers);
                std::string rating = nlohmann::json::parse(response.body)["maybe_rating_value"];
                if (rating == "positive")
                    return Rating::Positive;
                if (rating == "neutral")
                    return Rating::Neutral;
                if (rating == "negative")
                    return Rating::Negative;
                else
                    return Rating::Unknown;
            }
            catch (const std::exception e)
            {
                return Rating::Unknown;
            }
        }
        bool SetMyRating(Rating rating = Rating::Neutral)
        {
            if (!authCookie.size())
                return false;

            enforceRateLimit();

            std::string convertedRating = "neutral";
            if (rating == Rating::Positive)
                convertedRating = "positive";
            else if (rating == Rating::Negative)
                convertedRating = "negative";

            std::map<std::string, std::string> headers = {
                { "Content-Type", "application/json" },
                { "Accept", "application/json" }
            };

            if (authCookie.size())
                headers.insert({
                    { "Credentials", "include" },
                    { "Cookie", std::string("session=" + authCookie) }
                });

            try 
            {
                std::string data = "{\"entity_type\": \"tts_model\", \"entity_token\":\"" + this->modelToken + "\",\"rating_value\":\"" + convertedRating + "\"}";
                http::Response response = http::request(baseURL + "/v1/user_rating/rate", "POST", headers, data);
                return nlohmann::json::parse(response.body)["success"];
            }
            catch (const std::exception e) {  }
            return false;
        }
        bool Refetch()
        {
            enforceRateLimit();

            std::map<std::string, std::string> headers = {
                { "Content-Type", "application/json" },
                { "Accept", "application/json" }
            };

            if (authCookie.size())
                headers.insert({
                    { "Credentials", "include" },
                    { "Cookie", std::string("session=" + authCookie) }
                });

            http::Response res = http::request(baseURL + "/tts/list", "GET", headers);
            nlohmann::json body = nlohmann::json::parse(res.body);

            try {
                for (const auto& modelData : body["models"]) {
                    if (modelData["model_token"] == this->modelToken)
                    {
                        this->modelToken = modelData["model_token"];
                        this->ttsModelType = modelData["tts_model_type"];
                        this->creatorUserToken = modelData["creator_user_token"];
                        this->creatorUsername = modelData["creator_username"];
                        this->creatorDisplayName = modelData["creator_display_name"];
                        this->creatorGravatarHash = modelData["creator_gravatar_hash"];
                        this->title = modelData["title"];
                        this->ietfLanguageTag = modelData["ietf_language_tag"];
                        this->ietfPrimaryLanguageSubtag = modelData["ietf_primary_language_subtag"];
                        this->isFrontPageFeatured = modelData["is_front_page_featured"];
                        this->isTwitchFeatured = modelData["is_twitch_featured"];
                        this->maybeSuggestedUniqueBotCommand = modelData["maybe_suggested_unique_bot_command"];
                        this->creatorSetVisibility = modelData["creator_set_visibility"];
                        this->userRatings.positiveCount = modelData["user_ratings"]["positive_count"];
                        this->userRatings.negativeCount = modelData["user_ratings"]["negative_count"];
                        this->userRatings.totalCount = modelData["user_ratings"]["total_count"];

                        // Decode the category_tokens array
                        for (const auto& category : modelData["category_tokens"]) {
                            this->categoryTokens.push_back(category);
                        }

                        this->createdAt = modelData["created_at"];
                        this->updatedAt = modelData["updated_at"];

                        return true;
                    }
                }
            }
            catch (const std::exception e) {  }

            return false;
        }

        // actual model details
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
    };

    class TTS {
    public:
        static TTSModel* GetModelByToken(std::string token)
        {
            auto models = GetModels();
            for (int i = 0; i < models.size(); i++) {
                if (models[i]->modelToken == token)
                {
                    return models[i];
                }
            }

            return nullptr;
        }
        static TTSModel* GetModelByTitle(std::string title)
        {
            auto models = GetModels();
            for (int i = 0; i < models.size(); i++) {
                if (models[i]->title == title)
                    return models[i];
            }

            return nullptr;
        }
        static std::vector<TTSModel*> GetModels()
        {
            enforceRateLimit();

            std::vector<TTSModel*> models;

            std::map<std::string, std::string> headers = {
                { "Content-Type", "application/json" },
                { "Accept", "application/json" }
            };

            if (authCookie.size())
                headers.insert({
                    { "Credentials", "include" },
                    { "Cookie", std::string("session=" + authCookie) }
                });

            http::Response res = http::request(baseURL + "/tts/list", "GET", headers);
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
            catch (const std::exception e) {  }

            return models;
        }
    };
}