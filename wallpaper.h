#ifndef WALLPAPER_H
#define WALLPAPER_H

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

struct RunResult {
    bool ok;
    std::string text;
};

class Wallpaper {
   private:
    std::string name;
    std::string path;
    static RunResult runShellCommand(const std::string& command);
    static RunResult runAppleScript(const std::vector<std::string>& applescripts);
    static void clearDirectory(const fs::path& filepath);
    static bool downloadFile(const std::string& url, const fs::path& filepath);
    static size_t downloadWriteCallback(void* contents, size_t size, size_t nmemb, std::string* data);

   public:
    static std::vector<Wallpaper> get();
    static bool set(const std::string& url, const fs::path& path);
    Wallpaper(std::string  name, std::string  path) : name(std::move(name)), path(std::move(path)){};
    [[nodiscard]] std::string getName() const;
    [[nodiscard]] std::string getPath() const;
};

#endif