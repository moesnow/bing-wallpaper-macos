#ifndef BING_H
#define BING_H

#include <string>

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
    static bool checkConnection(const std::string& countrycode);
    explicit Bing(const std::string& countrycode);
    [[nodiscard]] std::string getName() const;
    [[nodiscard]] std::string getUrl() const;
};

#endif