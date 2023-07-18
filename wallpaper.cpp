#include <wallpaper.h>

RunResult runShellCommand(const std::string& command) {
    RunResult result;
    result.ok = false;
    result.text = "";

    FILE* pipe = popen(command.c_str(), "r");

    if (!pipe) {
        std::cerr << "Failed to execute command" << std::endl;
        return result;
    }

    const int bufferSize = 128;
    char buffer[bufferSize];
    while (fgets(buffer, bufferSize, pipe) != nullptr) {
        result.text += buffer;
    }
    int returnCode = pclose(pipe);
    result.ok = (returnCode == 0);

    return result;
}

RunResult runAppleScript(const std::vector<std::string>& applescripts) {
    std::string command = "";
    for (std::string script : applescripts) {
        command += " -e '" + script + "'";
    }
    command = "osascript" + command + " 2>&1";
    return runShellCommand(command);
}

void clearDirectory(const fs::path& filepath) {
    std::vector<fs::path> jpgFiles;
    for (const auto& entry : fs::directory_iterator(filepath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".jpg") {
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

// Callback functions
size_t downloadWriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    size_t totalSize = size * nmemb;
    data->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

bool downloadFile(const std::string& url, const fs::path& filepath) {
    create_directories(filepath.parent_path());
    std::ifstream file(filepath);
    if (file.good()) {
        return true;
    } else {
        clearDirectory(filepath.parent_path());
    }

    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        std::string buffer;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, downloadWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            std::ofstream outputFile(filepath, std::ios::binary);
            outputFile.write(buffer.c_str(), buffer.size());
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