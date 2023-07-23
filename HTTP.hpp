#pragma once

#include <curl/curl.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>

namespace http {
    struct Response {
        long status_code;
        curl_slist* cookies;
        std::map<std::string, std::string> headers;
        std::string body;
    };

    size_t WriteHeaderCallback(void* contents, size_t size, size_t nmemb, std::map<std::string, std::string>* headers) {
        size_t total_size = size * nmemb;
        std::string header_line(static_cast<char*>(contents), total_size);

        // Split the header line into key and value
        size_t separatorPos = header_line.find(':');
        if (separatorPos != std::string::npos) {
            std::string key = header_line.substr(0, separatorPos);
            std::string value = header_line.substr(separatorPos + 2); // +2 to skip the space after colon
            headers->emplace(key, value);
        }

        return total_size;
    }

    size_t WriteBodyCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
        size_t total_size = size * nmemb;
        output->append(static_cast<char*>(contents), total_size);
        return total_size;
    }

    std::vector<char> downloadWavFile(const std::string& url) {
        std::vector<char> buffer;

        CURL* curl = curl_easy_init();
        if (curl) {
            // Set the URL to download
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            // Set the write function to save the downloaded data into the buffer
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
                size_t totalSize = size * nmemb;
                std::vector<char>* buffer = static_cast<std::vector<char>*>(userdata);
                buffer->insert(buffer->end(), static_cast<char*>(ptr), static_cast<char*>(ptr) + totalSize);
                return totalSize;
            });
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

            // Perform the download
            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                std::cerr << "Failed to download the WAV file. Error: " << curl_easy_strerror(res) << std::endl;
            }

            // Clean up curl
            curl_easy_cleanup(curl);
        }
        else {
            std::cerr << "Failed to initialize curl." << std::endl;
        }

        return buffer;
    }

    std::string encode(const std::string data) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            std::cerr << "cURL initialization failed!" << std::endl;
            return "";
        }

        char* encodedData = curl_easy_escape(curl, data.c_str(), static_cast<int>(data.length()));
        std::string result(encodedData);
        curl_free(encodedData);
        curl_easy_cleanup(curl);

        return result;
    }

    Response request(
        const std::string url, 
        const std::string method,
        const std::map<std::string, std::string> headers = {},
        const std::string data = ""
    )
    {
        CURL* curl;
        CURLcode res;
        Response response;

        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteHeaderCallback);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response.headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteBodyCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);

            // Set custom headers
            struct curl_slist* headerList = nullptr;
            for (const auto& header : headers) {
                std::string header_line = header.first + ": " + header.second;
                headerList = curl_slist_append(headerList, header_line.c_str());
            }
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

            // Set POST/PUT data
            if (!data.empty()) {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
            }

            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                std::cerr << "Error during cURL request: " << curl_easy_strerror(res) << std::endl;
            }

            // Get response
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status_code);
            curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &response.cookies);

            // Clean up
            curl_slist_free_all(headerList);
            curl_easy_cleanup(curl);
        }

        return response;
    }
}