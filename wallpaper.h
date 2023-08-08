#ifndef WALLPAPER_H
#define WALLPAPER_H

#include <curl/curl.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

struct RunResult {
    bool ok;
    std::string text;
};

class Wallpaper {
   private:
    std::string name;
    std::string path;
    static RunResult runShellCommand(const std::string& command);
    static RunResult runAppleScript(const std::vector<std::string>& scripts);
    static void clearDirectory(const fs::path& filepath);
    static bool downloadFile(const std::string& url, const fs::path& filepath);
    static size_t downloadWriteCallback(void* contents, size_t size, size_t nmemb, std::string* data);

   public:
    static std::vector<Wallpaper> get();
    static bool set(const std::string url, const fs::path path);
    Wallpaper(const std::string& name, const std::string& path) : name(name), path(path){};
    std::string getName() const;
    std::string getPath() const;
};

#endif