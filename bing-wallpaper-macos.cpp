#include <bing.h>
#include <curl/curl.h>
#include <wallpaper.h>

#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;
using namespace std::filesystem;
using json = nlohmann::json;

// Function declarations
void printHelp();
void printVersion();
void printUnknown(const string arg);
void printSuccess(const string date);
string getDate();
json getDefaultConfig();
json getConfig(const path configpath);
bool checkNeedRun(const json configJson);
void updateConfig(const path configpath, const json jsonData);

// Constants
const string ProgramName = "bing-wallpaper-macos";
const string ProgramVersion = "0.0.8";
const path ProgramDir = string(getenv("HOME")) + "/.local/" + ProgramName + "/";
const string ConfigName = "config.json";

// Main function
int main(int argc, char* argv[]) {
    json config = getConfig(ProgramDir / ConfigName);

    if (argc > 1) {
        string arg(argv[1]);
        if (arg == "--auto") {
            if (!checkNeedRun(config)) {
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

    if (!Bing::checkConnection(config["country_code"])) {
        cerr << "Failed to connect to destination URL" << endl;
        return 1;
    }

    Bing bing(config["country_code"]);

    if (Wallpaper::set(bing.getUrl(), ProgramDir / bing.getName())) {
        printSuccess(getDate());
    } else {
        cerr << "Failed to apply wallpaper" << endl;
        return 1;
    }

    config["wallpaper_name"] = bing.getName();
    config["last_update_date"] = getDate();
    updateConfig(ProgramDir / ConfigName, config);

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

void printUnknown(const string arg) {
    cout << ProgramName + ": option " << arg << " is unknown.\n"
         << ProgramName + ": try '" + ProgramName + " --help' for more information.\n";
}

void printSuccess(const string date) {
    cout << "\033[0m[\033[33m" << date << "\033[0m] "
         << "\033[35mWallpaper applied successfully :)" << endl;
}

string getDate() {
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[11];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    mktime(timeinfo);

    strftime(buffer, sizeof(buffer), "%Y/%-m/%-d", timeinfo);
    return string(buffer);
}

json getDefaultConfig() {
    json jsonData;
    jsonData["last_update_date"] = "";
    jsonData["wallpaper_name"] = "";
    jsonData["country_code"] = "";
    return jsonData;
}

json getConfig(const path configPath) {
    ifstream configFile(configPath);
    if (configFile.is_open()) {
        try {
            json configJson;
            configFile >> configJson;
            if (!configJson.contains("last_update_date")) {
                configJson["last_update_date"] = "";
            }
            if (!configJson.contains("wallpaper_name")) {
                configJson["wallpaper_name"] = "";
            }
            if (!configJson.contains("country_code")) {
                configJson["country_code"] = "";
            }
            return configJson;
        } catch (const exception& e) {
            cerr << "Error: Failed to parse config file. " << e.what() << endl;
        }
    }
    configFile.close();
    return getDefaultConfig();
}

bool checkNeedRun(const json configJson) {
    bool isDateExpired = getDate() == configJson["last_update_date"] ? false : true;
    bool isWallpaperChange = false;
    string wallpaperName = configJson["wallpaper_name"];
    const vector<Wallpaper> wallpapers = Wallpaper::get();
    for (const auto& wallpaper : wallpapers) {
        if (wallpaper.getPath().find(wallpaperName) == std::string::npos) {
            isWallpaperChange = true;
            break;
        }
    }
    return isDateExpired || isWallpaperChange ? true : false;
}

void updateConfig(const path configpath, const json jsonData) {
    create_directories(configpath.parent_path());
    ofstream file(configpath);
    if (file.is_open()) {
        file << jsonData.dump(4);
        file.close();
    } else {
        cerr << "Failed to open file" << endl;
    }
}