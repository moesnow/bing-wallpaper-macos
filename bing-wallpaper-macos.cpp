#include <curl/curl.h>

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

// Function declarations
void printHelp();
void printVersion();
void printUnknown(const string& arg);
string getYesterdayDate();
bool check_run_today(const string& configPath);
bool checkNetworkConnection(const string& url);
string getJsonContent(const string& url);
string replaceAndAddPrefix(const string& url, const string& target, const string& replacement, const string& prefix);
string getTemporaryFilePath();
void clearDirectory(const string& path);
string downloadFile(const string& url, const string& filedir, const string& filename);
bool runCommand(const string& script);
void updateConfig(const string& configDir, const string& configName);

// Constants
const string ProgramName = "bing-wallpaper-macos";
const string ProgramVersion = "0.0.3";
const string ConfigDir = string(getenv("HOME")) + "/.config/" + ProgramName + "/";
const string DownloadDir = string(getenv("HOME")) + "/.local/" + ProgramName + "/";
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
            if (check_run_today(ConfigDir + ConfigName)) {
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
    string downloadedFilePath = downloadFile(modifiedUrl, DownloadDir, fileName);
    if (downloadedFilePath.empty()) {
        cerr << "Failed to download wallpaper" << endl;
        return 1;
    }

    string appleScript1 = "tell application \"System Events\" to tell every desktop to set picture to \"" + downloadedFilePath + "\"";
    string appleScript2 = "tell application \"System Events\" to if picture of item 1 of desktops does not contain \"" + downloadedFilePath + "\" then error";
    string shellCommand = "osascript -e '" + appleScript1 + "' -e '" + appleScript2 + "'";
    if (runCommand(shellCommand)) {
        cout << "\033[0m[\033[33m" << getYesterdayDate() << "\033[0m] "
             << "\033[35mWallpaper applied successfully :)" << endl;
    } else {
        cerr << "Failed to apply wallpaper" << endl;
        return 1;
    }

    updateConfig(ConfigDir, ConfigName);

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

bool check_run_today(const string& configPath) {
    ifstream configFile(configPath);
    if (configFile.is_open()) {
        json configJson;
        configFile >> configJson;
        if (configJson.contains("last_update_date")) {
            if (getYesterdayDate() == configJson["last_update_date"]) {
                configFile.close();
                return true;
            }
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

void clearDirectory(const string& path) {
    string command = "rm -rf \"" + path + "\"/*.jpg";
    int status = system(command.c_str());
    if (status != 0) {
        cerr << "Failed to clear directory: " << path << endl;
    }
}

string downloadFile(const string& url, const string& filedir, const string& filename) {
    string createDirCommand = "mkdir -p " + filedir;
    system(createDirCommand.c_str());
    string FilePath = filedir + filename;
    ifstream file(FilePath);
    if (file.good()) {
        // cout << "file exists, skip download" << endl;
        return FilePath;
    } else {
        clearDirectory(filedir);
    }

    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        string buffer;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            ofstream outputFile(FilePath, ios::binary);
            outputFile.write(buffer.c_str(), buffer.size());
            outputFile.close();
        } else {
            FilePath.clear();
            cerr << "Failed to download file: " << curl_easy_strerror(res) << endl;
        }

        curl_easy_cleanup(curl);
    } else {
        FilePath.clear();
        cerr << "Failed to initialize libcurl" << endl;
    }

    return FilePath;
}

bool runCommand(const string& command) {
    int result = system(command.c_str());
    return !result;
}

void updateConfig(const string& configDir, const string& configName) {
    string createDirCommand = "mkdir -p " + configDir;
    system(createDirCommand.c_str());

    json jsonData;
    jsonData["last_update_date"] = getYesterdayDate();

    string filePath = configDir + configName;
    ofstream file(filePath);
    if (file.is_open()) {
        file << jsonData.dump(4);
        file.close();
    } else {
        cerr << "Failed to open file" << endl;
    }
}
