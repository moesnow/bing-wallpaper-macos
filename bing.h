#ifndef BING_H
#define BING_H

#include <curl/curl.h>

#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Bing {
   private:
    std::string name;
    std::string url;
    static bool checkNetworkConnection(const std::string& url);
    static std::string getJsonContent(const std::string& url);
    static std::string replaceAndAddPrefix(const std::string& url, const std::string& target, const std::string& replacement, const std::string& prefix);
    static size_t getWriteCallback(void* contents, size_t size, size_t nmemb, std::string* data);

   public:
    static const std::string BingHost;
    static const std::string BingPath;
    static bool checkConnection(const std::string countrycode);
    Bing(const std::string countrycode);
    std::string getName() const;
    std::string getUrl() const;
};

#endif