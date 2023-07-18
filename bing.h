#ifndef BING_H
#define BING_H

#include <curl/curl.h>

#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

const std::string BingHost = "https://www.bing.com";
const std::string BingPath = "/HPImageArchive.aspx?format=js&n=1&idx=0";

bool checkNetworkConnection(const std::string& url);
std::string getJsonContent(const std::string& url);
std::string replaceAndAddPrefix(const std::string& url, const std::string& target, const std::string& replacement, const std::string& prefix);

struct Pictures {
    std::string name;
    std::string url;
};

struct Bing {
    static bool checkConnection(const std::string countrycode) {
        std::string host = countrycode == "zh-cn" ? "https://cn.bing.com" : BingHost;
        return checkNetworkConnection(host);
    }
    static Pictures getPicture(const std::string countrycode) {
        std::string host = countrycode == "zh-cn" ? "https://cn.bing.com" : BingHost;
        Pictures picture;
        std::string wallpaperJsonContent = getJsonContent(host + BingPath + "&mkt=" + countrycode);
        json wallpaperjsonData = json::parse(wallpaperJsonContent);
        std::string wallpaperUrl = wallpaperjsonData["images"][0]["url"];
        picture.url = replaceAndAddPrefix(wallpaperUrl, "1920x1080", "UHD", host);
        picture.name = (std::string)wallpaperjsonData["images"][0]["startdate"] + "_" + (std::string)wallpaperjsonData["images"][0]["title"] + ".jpg";
        return picture;
    }
};

#endif