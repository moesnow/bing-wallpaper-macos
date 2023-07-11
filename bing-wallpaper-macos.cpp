#include <curl/curl.h>

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

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

    // 初始化libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // 创建CURL对象
    curl = curl_easy_init();
    if (curl) {
        // 设置要访问的URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // 设置不接收数据，只检查连接是否成功
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);

        // 执行HTTP请求
        res = curl_easy_perform(curl);

        // 检查连接是否成功
        if (res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        }

        // 清理CURL对象
        curl_easy_cleanup(curl);
    }

    // 清理libcurl
    curl_global_cleanup();

    // 判断网络连接是否正常
    return (response_code == 200);
}

size_t WriteCallback1(void* contents, size_t size, size_t nmemb, string* data) {
    size_t totalSize = size * nmemb;
    data->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

string getJsonContent(const string& url) {
    CURL* curl;
    CURLcode res;
    string content;

    // 初始化 libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // 创建 CURL 对象
    curl = curl_easy_init();
    if (curl) {
        // 设置要访问的 URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // 设置回调函数以接收数据
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback1);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);

        // 执行 HTTP 请求
        res = curl_easy_perform(curl);

        // 检查请求是否成功
        if (res != CURLE_OK) {
            cerr << "HTTP请求错误: " << curl_easy_strerror(res) << endl;
        }

        // 清理 CURL 对象
        curl_easy_cleanup(curl);
    }

    // 清理 libcurl
    curl_global_cleanup();

    return content;
}

string replaceAndAddPrefix(const string& url, const string& target, const string& replacement, const string& prefix) {
    string modifiedUrl = url;
    size_t pos = 0;

    // 查找并替换所有的目标字符串
    while ((pos = modifiedUrl.find(target, pos)) != string::npos) {
        modifiedUrl.replace(pos, target.length(), replacement);
        pos += replacement.length();
    }

    // 添加前缀
    return prefix + modifiedUrl;
}

string getTemporaryFilePath() {
    const char* tmpdir = getenv("TMPDIR");
    if (tmpdir == nullptr) {
        tmpdir = "/tmp/";
    }
    string tempDir = string(tmpdir) + "bing-wallpaper/";

    // 创建临时文件夹
    string createDirCommand = "mkdir -p " + tempDir;
    system(createDirCommand.c_str());
    return string(tempDir);
}

size_t WriteCallback2(void* contents, size_t size, size_t nmemb, string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

void clearDirectory(const string& path) {
    string command = "rm -rf \"" + path + "\"/*";
    int status = system(command.c_str());
    if (status != 0) {
        cerr << "Failed to clear directory: " << path << endl;
    }
}

string downloadFile(const string& url, const string& filename) {
    // 创建一个临时文件路径
    string tempPath = getTemporaryFilePath();
    string tempFilePath = tempPath + filename;
    ifstream file(tempFilePath);
    if (file.good()) {
        cout << "文件存在，跳过下载" << endl;
        return tempFilePath;
    } else {
        // 下载前先清空目录
        string command = "rm -rf \"" + tempPath + "\"/*";
        system(command.c_str());
    }

    // 初始化 libcurl
    CURL* curl = curl_easy_init();
    if (curl) {
        // 设置 URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // 设置回调函数和数据存储位置
        string buffer;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback2);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        // 执行下载操作
        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            // 将下载的数据写入临时文件
            ofstream outputFile(tempFilePath, ios::binary);
            outputFile.write(buffer.c_str(), buffer.size());
            outputFile.close();
        } else {
            // 下载失败时，返回空字符串
            tempFilePath.clear();
            cerr << "Failed to download file: " << curl_easy_strerror(res) << endl;
        }

        // 清理 libcurl
        curl_easy_cleanup(curl);
    } else {
        // 初始化 libcurl 失败时，返回空字符串
        tempFilePath.clear();
        cerr << "Failed to initialize libcurl" << endl;
    }

    return tempFilePath;
}

bool runAppleScript(const string& script) {
    string command = "osascript -e '" + script + "'";
    int result = system(command.c_str());
    return !result;
}

void updateConfig(const string& configDir, const string& configName) {
    string createDirCommand = "mkdir -p " + configDir;
    system(createDirCommand.c_str());
    // 构建 JSON 数据
    json jsonData;
    jsonData["last_update_date"] = getYesterdayDate();
    // 将 JSON 数据写入文件
    string filePath = configDir + configName;
    ofstream file(filePath);
    if (file.is_open()) {
        file << jsonData.dump(4);  // 使用 4 个空格进行缩进
        file.close();
        cout << "JSON 数据已写入文件" << endl;
    } else {
        cout << "无法打开文件" << endl;
    }
}

int main() {
    // 配置文件路径
    string configDir = string(getenv("HOME")) + "/.config/bing-wallpaper-macos/";
    string configName = "config.json";

    // 判断今天是否运行过
    if (check_run_today(configDir + configName)) {
        return 0;
    }

    // 判断网络连接是否正常
    string check_network_url = "https://cn.bing.com";
    bool connected = checkNetworkConnection(check_network_url);
    if (connected) {
        cout << "网络连接正常" << endl;
    } else {
        cout << "无法连接到目标网址" << endl;
        return 0;
    }

    // Bing壁纸API
    string wallpaper_json_url = "https://cn.bing.com/HPImageArchive.aspx?format=js&n=1&idx=0";
    string wallpaperjsonContent = getJsonContent(wallpaper_json_url);

    // 解析 JSON
    json wallpaperjsonData = json::parse(wallpaperjsonContent);
    // 提取 images 字段中的 url
    string wallpaperUrl = wallpaperjsonData["images"][0]["url"];
    string modifiedUrl = replaceAndAddPrefix(wallpaperUrl, "1920x1080", "UHD", "https://cn.bing.com");
    // 提取 images 字段中的 startdate title
    string fileName = (string)wallpaperjsonData["images"][0]["startdate"] + "_" + (string)wallpaperjsonData["images"][0]["title"] + ".jpg";
    // 下载文件
    string downloadedFilePath = downloadFile(modifiedUrl, fileName);
    if (!downloadedFilePath.empty()) {
        cout << "文件下载完成" << endl;
    } else {
        cout << "文件下载失败" << endl;
        return 0;
    }

    // 调用AppleScript应用壁纸
    string script = "tell application \"System Events\" to tell every desktop to set picture to \"" + downloadedFilePath + "\"";
    int result = runAppleScript(script);
    if (result) {
        cout << "壁纸应用成功" << endl;
    } else {
        cout << "壁纸应用失败" << endl;
        return 0;
    }

    // 更新配置文件
    updateConfig(configDir, configName);

    return 0;
}
