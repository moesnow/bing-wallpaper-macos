#include <curl/curl.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;
using namespace std::filesystem;
using json = nlohmann::json;

// Function declarations
void printHelp();
void printVersion();
void printUnknown(const string& arg);
string getYesterdayDate();
bool checkRunToday(const path& configpath);
bool checkNetworkConnection(const string& url);
string getJsonContent(const string& url);
string replaceAndAddPrefix(const string& url, const string& target, const string& replacement, const string& prefix);
void clearDirectory(const path& filepath);
bool downloadFile(const string& url, const path& filepath);
bool runShellCommand(const string& script);
void updateConfig(const path& configpath);

// Constants
const string ProgramName = "bing-wallpaper-macos";
const string ProgramVersion = "0.0.5";
const path ProgramDir = string(getenv("HOME")) + "/.local/" + ProgramName + "/";
const string ConfigName = "config.json";
const string CheckNetworkUrl = "https://cn.bing.com";
const string WallpaperJsonUrl = CheckNetworkUrl + "/HPImageArchive.aspx?format=js&n=1&idx=0";

// Callback functions
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* data) {
    size_t totalSize = size * nmemb;
    data->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// Main function
int main(int argc, char* argv[]) {
    if (argc > 1) {
        string arg(argv[1]);
        if (arg == "--auto") {
            if (checkRunToday(ProgramDir / ConfigName)) {
                return 0;
            }
        } else if (arg == "--help") {
            printHelp();
            return 0;
        } else if (arg == "--version") {
            printVersion();
            return 0;
        } else {
            printUnknown(arg);
            return 1;
        }
    }

    if (!checkNetworkConnection(CheckNetworkUrl)) {
        cerr << "Failed to connect to destination URL" << endl;
        return 1;
    }

    string wallpaperJsonContent = getJsonContent(WallpaperJsonUrl);
    json wallpaperjsonData = json::parse(wallpaperJsonContent);
    string wallpaperUrl = wallpaperjsonData["images"][0]["url"];
    string modifiedUrl = replaceAndAddPrefix(wallpaperUrl, "1920x1080", "UHD", "https://cn.bing.com");
    string fileName = (string)wallpaperjsonData["images"][0]["startdate"] + "_" + (string)wallpaperjsonData["images"][0]["title"] + ".jpg";
    path downloadedFilePath = ProgramDir / fileName;
    if (downloadFile(modifiedUrl, downloadedFilePath)) {
        cerr << "Failed to download wallpaper" << endl;
        return 1;
    }

    string appleScript1 = "tell application \"System Events\" to tell every desktop to set picture to \"" + downloadedFilePath.string() + "\"";
    string appleScript2 = "tell application \"System Events\" to if picture of item 1 of desktops does not contain \"" + downloadedFilePath.string() + "\" then error";
    string shellCommand = "osascript -e '" + appleScript1 + "' -e '" + appleScript2 + "'";
    if (runShellCommand(shellCommand)) {
        cout << "\033[0m[\033[33m" << getYesterdayDate() << "\033[0m] "
             << "\033[35mWallpaper applied successfully :)" << endl;
    } else {
        cerr << "Failed to apply wallpaper" << endl;
        return 1;
    }

    updateConfig(ProgramDir / ConfigName);

    return 0;
}

// Utility functions
void printHelp() {
    cout << "Usage: " << ProgramName << " [options]\n"
         << "Options:\n"
         << "  --auto     : Check if program run today\n"
         << "  --version  : Display program version\n"
         << "  --help     : Display this help message\n";
}

void printVersion() {
    cout << "Program version: " << ProgramVersion << endl;
}

void printUnknown(const string& arg) {
    cout << ProgramName + ": option " << arg << " is unknown.\n"
         << ProgramName + ": try '" + ProgramName + " --help' for more information.\n";
}

string getYesterdayDate() {
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[9];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    timeinfo->tm_mday--;
    mktime(timeinfo);

    strftime(buffer, sizeof(buffer), "%Y%m%d", timeinfo);
    return string(buffer);
}

bool checkRunToday(const path& configPath) {
    ifstream configFile(configPath);
    if (configFile.is_open()) {
        try {
            json configJson;
            configFile >> configJson;
            if (configJson.contains("last_update_date")) {
                if (getYesterdayDate() == configJson["last_update_date"]) {
                    configFile.close();
                    return true;
                }
            }
        } catch (const exception& e) {
            cerr << "Error: Failed to parse config file. " << e.what() << endl;
        }
    }
    configFile.close();
    return false;
}

bool checkNetworkConnection(const string& url) {
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

string getJsonContent(const string& url) {
    CURL* curl;
    CURLcode res;
    string content;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            cerr << "HTTP request error: " << curl_easy_strerror(res) << endl;
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return content;
}

string replaceAndAddPrefix(const string& url, const string& target, const string& replacement, const string& prefix) {
    string modifiedUrl = url;
    size_t pos = 0;

    while ((pos = modifiedUrl.find(target, pos)) != string::npos) {
        modifiedUrl.replace(pos, target.length(), replacement);
        pos += replacement.length();
    }

    return prefix + modifiedUrl;
}

void clearDirectory(const path& filepath) {
    for (const auto& entry : directory_iterator(filepath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".jpg") {
            remove(entry);
        }
    }
}

bool downloadFile(const string& url, const path& filepath) {
    create_directories(filepath.parent_path());
    ifstream file(filepath);
    if (file.good()) {
        // cout << "file exists, skip download" << endl;
        return false;
    } else {
        clearDirectory(filepath.parent_path());
    }

    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        string buffer;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            ofstream outputFile(filepath, ios::binary);
            outputFile.write(buffer.c_str(), buffer.size());
            outputFile.close();
        } else {
            cerr << "Failed to download file: " << curl_easy_strerror(res) << endl;
            return true;
        }
        curl_easy_cleanup(curl);
    } else {
        cerr << "Failed to initialize libcurl" << endl;
        return true;
    }

    return false;
}

bool runShellCommand(const string& command) {
    int result = system(command.c_str());
    return !result;
}

void updateConfig(const path& configPath) {
    create_directories(configPath.parent_path());
    json jsonData;
    jsonData["last_update_date"] = getYesterdayDate();

    ofstream file(configPath);
    if (file.is_open()) {
        file << jsonData.dump(4);
        file.close();
    } else {
        cerr << "Failed to open file" << endl;
    }
}
