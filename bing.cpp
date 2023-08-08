#include <bing.h>

const std::string Bing::BingHost = "https://www.bing.com";
const std::string Bing::BingPath = "/HPImageArchive.aspx?format=js&n=1&idx=0";

bool Bing::checkNetworkConnection(const std::string& url) {
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

bool Bing::checkConnection(const std::string countrycode) {
    std::string host = countrycode == "zh-cn" ? "https://cn.bing.com" : Bing::BingHost;
    return checkNetworkConnection(host);
}

std::string Bing::getJsonContent(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string content;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Bing::checkConnection);
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

std::string Bing::replaceAndAddPrefix(const std::string& url, const std::string& target, const std::string& replacement, const std::string& prefix) {
    std::string modifiedUrl = url;
    size_t pos = 0;

    while ((pos = modifiedUrl.find(target, pos)) != std::string::npos) {
        modifiedUrl.replace(pos, target.length(), replacement);
        pos += replacement.length();
    }

    return prefix + modifiedUrl;
}

size_t Bing::getWriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    size_t totalSize = size * nmemb;
    data->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

Bing::Bing(const std::string countrycode) {
    std::string host = countrycode == "zh-cn" ? "https://cn.bing.com" : BingHost;
    std::string wallpaperJsonContent = getJsonContent(host + BingPath + "&mkt=" + countrycode);
    json wallpaperjsonData = json::parse(wallpaperJsonContent);
    std::string wallpaperUrl = wallpaperjsonData["images"][0]["url"];
    url = replaceAndAddPrefix(wallpaperUrl, "1920x1080", "UHD", host);
    std::string picturTitle = (std::string)wallpaperjsonData["images"][0]["title"];
    picturTitle.erase(std::remove(picturTitle.begin(), picturTitle.end(), '/'), picturTitle.end());
    name = (std::string)wallpaperjsonData["images"][0]["startdate"] + "_" + picturTitle + ".jpg";
}

std::string Bing::getName() const {
    return name;
}

std::string Bing::getUrl() const {
    return url;
}