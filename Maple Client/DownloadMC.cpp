#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

bool DownloadFile(const std::string& url, const std::string& filepath) {
    fs::create_directories(fs::path(filepath).parent_path());

    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK) {
            std::ofstream outfile(filepath, std::ios::binary);
            outfile.write(readBuffer.c_str(), readBuffer.size());
            outfile.close();
            return true;
        }
    }
    return false;
}

void DownloadMinecraft() {
    const std::string version = "1.12.2";
    const std::string versionManifestUrl = "https://launchermeta.mojang.com/mc/game/version_manifest.json";
    const std::string minecraftDir = ".minecraft/";

    fs::create_directories(minecraftDir);

    std::string manifestJson;
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, versionManifestUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &manifestJson);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    json manifest = json::parse(manifestJson);
    std::string versionUrl;
    for (const auto& v : manifest["versions"]) {
        if (v["id"] == version) {
            versionUrl = v["url"];
            break;
        }
    }

    if (versionUrl.empty()) {
        std::cout << "Version not found!" << std::endl;
        return;
    }

    std::string versionJson;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, versionUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &versionJson);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    json versionInfo = json::parse(versionJson);

    std::string clientUrl = versionInfo["downloads"]["client"]["url"];
    DownloadFile(clientUrl, minecraftDir + "versions/" + version + "/" + version + ".jar");

    std::ofstream versionJsonFile(minecraftDir + "versions/" + version + "/" + version + ".json");
    versionJsonFile << versionJson;
    versionJsonFile.close();

    std::string assetIndexUrl = versionInfo["assetIndex"]["url"];
    std::string assetIndexJson;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, assetIndexUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &assetIndexJson);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    json assetIndex = json::parse(assetIndexJson);

    std::string assetIndexVersion = versionInfo["assetIndex"]["id"];
    std::ofstream assetIndexFile(minecraftDir + "assets/indexes/" + assetIndexVersion + ".json");
    assetIndexFile << assetIndexJson;
    assetIndexFile.close();

    for (const auto& [assetName, assetInfo] : assetIndex["objects"].items()) {
        std::string hash = assetInfo["hash"];
        std::string assetUrl = "https://resources.download.minecraft.net/" + hash.substr(0, 2) + "/" + hash;
        std::string assetPath = minecraftDir + "assets/objects/" + hash.substr(0, 2) + "/" + hash;

        DownloadFile(assetUrl, assetPath);
    }

    for (const auto& library : versionInfo["libraries"]) {
        if (!library.contains("downloads")) {
            std::cout << "Warning: Library entry does not contain 'downloads' field. Skipping." << std::endl;
            continue;
        }

        const auto& downloads = library["downloads"];

        if (downloads.contains("artifact")) {
            const auto& artifact = downloads["artifact"];
            if (artifact.contains("url") && artifact.contains("path")) {
                std::string url = artifact["url"];
                std::string path = artifact["path"];
                std::string fullPath = minecraftDir + "libraries/" + path;

                std::cout << "Downloading library: " << path << std::endl;
                if (!DownloadFile(url, fullPath)) {
                    std::cerr << "Failed to download library: " << url << std::endl;
                }
            }
            else {
                std::cout << "Warning: Artifact entry is missing 'url' or 'path'. Skipping." << std::endl;
            }
        }

        if (downloads.contains("classifiers")) {
            const auto& classifiers = downloads["classifiers"];
            if (classifiers.contains("natives-windows")) {
                const auto& nativeInfo = classifiers["natives-windows"];
                if (nativeInfo.contains("url") && nativeInfo.contains("path")) {
                    std::string nativeUrl = nativeInfo["url"];
                    std::string nativePath = nativeInfo["path"];

                    fs::path fullPath(nativePath);
                    std::string fileName = fullPath.filename().string();
                    if (fileName.substr(fileName.length() - 4) == ".dll") {
                        std::string fullNativePath = minecraftDir + "versions/" + version + "/" + version + "-natives/" + fileName;

                        std::cout << "Downloading native library: " << fileName << std::endl;
                        if (!DownloadFile(nativeUrl, fullNativePath)) {
                            std::cerr << "Failed to download native library: " << nativeUrl << std::endl;
                        }
                    }
                    else {
                        std::cout << "Skipping non-DLL native file: " << fileName << std::endl;
                    }
                }
                else {
                    std::cout << "Warning: Native entry is missing 'url' or 'path'. Skipping." << std::endl;
                }
            }
        }

        if (library.contains("rules")) {
            bool shouldDownload = false;
            for (const auto& rule : library["rules"]) {
                if (rule.contains("action") && rule["action"] == "allow") {
                    if (rule.contains("os") && rule["os"].contains("name") && rule["os"]["name"] == "windows") {
                        shouldDownload = true;
                        break;
                    }
                }
            }
            if (!shouldDownload) {
                std::cout << "Skipping library due to rules: " << library["name"] << std::endl;
                continue;
            }
        }
    }

    std::string nativesUrl = "https://github.com/Douyin-vbuser/Minecraft-Genshin-Mod/releases/download/environment/1.12.2-natives.exe";
    std::string nativesPath = fs::absolute(minecraftDir + "versions/" + version + "/natives-" + version + ".exe").string();

    std::cout << "Downloading natives: " << nativesUrl << std::endl;
    if (DownloadFile(nativesUrl, nativesPath)) {
        std::cout << "Extracting natives..." << std::endl;
        std::string extractDir = fs::absolute(minecraftDir + "versions/" + version + "/" + version + "-natives").string();
        std::string extractCommand = "\"" + nativesPath + "\" -y -o\"" + extractDir + "\"";
        int result = std::system(extractCommand.c_str());

        if (result == 0) {
            std::cout << "Natives extracted successfully." << std::endl;
            fs::remove(nativesPath);
        }
        else {
            std::cerr << "Failed to extract natives. Error code: " << result << std::endl;
        }
    }
    else {
        std::cerr << "Failed to download natives." << std::endl;
    }

    std::cout << "Minecraft " << version << " and its resources have been downloaded successfully!" << std::endl;
}

int main() {
    DownloadMinecraft();
    return 0;
}