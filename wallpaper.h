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

RunResult runShellCommand(const std::string& command);
RunResult runAppleScript(const std::vector<std::string>& scripts);
void clearDirectory(const fs::path& filepath);
bool downloadFile(const std::string& url, const fs::path& filepath);

struct Screen {
    std::string name;
    std::string path;
};

struct Wallpaper {
    static std::vector<Screen> get() {
        std::string appleScript = R"delimiter(tell application "System Events"
    set outputText to "{"
    set isFirstDesktop to true
    repeat with current_desktop in desktops
        if isFirstDesktop then
            set outputText to outputText & "\"" & name of current_desktop & "\"" & ":" & "\"" & picture of current_desktop & "\""
            set isFirstDesktop to false
        else
            set outputText to outputText & "," & "\"" & name of current_desktop & "\"" & ":" & "\"" & picture of current_desktop & "\""
        end if	
    end repeat
    set outputText to outputText & "}"
end tell
return outputText)delimiter";
        RunResult result = runAppleScript({appleScript});
        std::vector<Screen> screens = {};
        if (result.ok) {
            json jsonData = json::parse(result.text);
            for (auto it = jsonData.begin(); it != jsonData.end(); ++it) {
                Screen screen = {it.key(), it.value()};
                screens.push_back(screen);
            }
            return screens;
        }
        return screens;
    }
    static bool set(const std::string url, const fs::path path) {
        if (!downloadFile(url, path)) {
            std::cerr << "Failed to download wallpaper" << std::endl;
            return false;
        }
        std::string appleScript1 = "tell application \"System Events\" to tell every desktop to set picture to \"" + path.string() + "\"";
        std::string appleScript2 = "tell application \"System Events\" to if picture of item 1 of desktops does not contain \"" + path.string() + "\" then error";
        return runAppleScript({appleScript1, appleScript2}).ok;
    }
};

#endif