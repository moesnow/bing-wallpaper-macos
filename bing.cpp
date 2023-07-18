#include <bing.h>

bool checkNetworkConnection(const std::string& url) {
    CURL* curl;
    CURLcode res;
    long response_code = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);

        res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return (response_code == 200);
}

// Callback functions
size_t getWriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    size_t totalSize = size * nmemb;
    data->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

std::string getJsonContent(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string content;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "HTTP request error: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return content;
}

std::string replaceAndAddPrefix(const std::string& url, const std::string& target, const std::string& replacement, const std::string& prefix) {
    std::string modifiedUrl = url;
    size_t pos = 0;

    while ((pos = modifiedUrl.find(target, pos)) != std::string::npos) {
        modifiedUrl.replace(pos, target.length(), replacement);
        pos += replacement.length();
    }

    return prefix + modifiedUrl;
}
