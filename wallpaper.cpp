#include <curl/curl.h>
#include <wallpaper.h>

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

RunResult Wallpaper::runShellCommand(const std::string& command) {
    RunResult result;
    result.ok = false;
    result.text = "";

    FILE* pipe = popen(command.c_str(), "r");

    if (!pipe) {
        std::cerr << "Failed to execute command" << std::endl;
        return result;
    }

    constexpr int bufferSize = 128;
    char buffer[bufferSize];
    while (fgets(buffer, bufferSize, pipe) != nullptr) {
        result.text += buffer;
    }
    const int returnCode = pclose(pipe);
    result.ok = (returnCode == 0);

    return result;
}

RunResult Wallpaper::runAppleScript(const std::vector<std::string>& applescripts) {
    std::string command;
    for (const std::string& script : applescripts) {
        command += " -e '" + script + "'";
    }
    command = "osascript" + command + " 2>&1";
    return runShellCommand(command);
}

void Wallpaper::clearDirectory(const fs::path& filepath) {
    std::vector<fs::path> jpgFiles;
    for (const auto& entry : fs::directory_iterator(filepath)) {
        if (entry.is_regular_file() && entry.path().extension().string() == ".jpg") {
            jpgFiles.push_back(entry);
        }
    }

    if (jpgFiles.size() > 30) {
        sort(jpgFiles.begin(), jpgFiles.end(), [](const fs::path& a, const fs::path& b) {
            return a.filename().string() < b.filename().string();
        });

        for (size_t i = 0; i < jpgFiles.size() - 30; ++i) {
            fs::remove(jpgFiles[i]);
        }
    }
}

bool Wallpaper::downloadFile(const std::string& url, const fs::path& filepath) {
    create_directories(filepath.parent_path());
    if (std::ifstream file(filepath); file.good()) {
        return true;
    } else {
        clearDirectory(filepath.parent_path());
    }

    if (CURL* curl = curl_easy_init(); curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        std::string buffer;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, downloadWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        if (CURLcode res = curl_easy_perform(curl); res == CURLE_OK) {
            std::ofstream outputFile(filepath, std::ios::binary);
            outputFile.write(buffer.c_str(), static_cast<std::streamsize>(buffer.size()));
            outputFile.close();
        } else {
            std::cerr << "Failed to download file: " << curl_easy_strerror(res) << std::endl;
            return false;
        }
        curl_easy_cleanup(curl);
    } else {
        std::cerr << "Failed to initialize libcurl" << std::endl;
        return false;
    }

    return true;
}

size_t Wallpaper::downloadWriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    const size_t totalSize = size * nmemb;
    data->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

std::vector<Wallpaper> Wallpaper::get() {
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
    auto [ok, text] = runAppleScript({appleScript});
    std::vector<Wallpaper> wallpapers = {};
    if (ok) {
        json jsonData = json::parse(text);
        for (auto it = jsonData.begin(); it != jsonData.end(); ++it) {
            wallpapers.emplace_back(it.key(), it.value());
        }
    }
    return wallpapers;
}

bool Wallpaper::set(const std::string& url, const fs::path& path) {
    if (!downloadFile(url, path)) {
        std::cerr << "Failed to download wallpaper" << std::endl;
        return false;
    }
    std::string appleScript1 = R"(tell application "System Events" to tell every desktop to set picture to ")" + path.string() + "\"";
    std::string appleScript2 = R"(tell application "System Events" to if picture of item 1 of desktops does not contain ")" + path.string() + "\" then error";
    return runAppleScript({appleScript1, appleScript2}).ok;
}

std::string Wallpaper::getName() const {
    return name;
}

std::string Wallpaper::getPath() const {
    return path;
}