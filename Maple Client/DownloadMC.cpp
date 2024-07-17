#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <zip.h>
#include <thread>

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

bool ExtractJar(const std::string& jarPath, const std::string& extractPath) {
    int err = 0;
    zip* z = zip_open(jarPath.c_str(), 0, &err);
    if (!z) {
        std::cerr << "Failed to open JAR file: " << jarPath << std::endl;
        return false;
    }

    for (int i = 0; i < zip_get_num_entries(z, 0); i++) {
        struct zip_stat st;
        zip_stat_index(z, i, 0, &st);
        char* contents = new char[st.size];

        zip_file* f = zip_fopen_index(z, i, 0);
        if (!f) {
            std::cerr << "Failed to open file in JAR: " << st.name << std::endl;
            delete[] contents;
            continue;
        }

        zip_fread(f, contents, st.size);
        zip_fclose(f);

        fs::path outPath = fs::path(extractPath) / st.name;
        fs::create_directories(outPath.parent_path());
        std::ofstream outFile(outPath, std::ios::binary);
        outFile.write(contents, st.size);
        outFile.close();

        delete[] contents;
    }

    zip_close(z);
    return true;
}

void CreateDummyLauncherProfiles(const std::string& filePath) {
    json dummyProfiles = {
        {"profiles", {
            {"Maple", {
                {"name", "Maple"},
                {"lastVersionId", "1.12.2-forge14.23.5.2854"},
                {"type", "custom"},
                {"lastUsed", "2023-06-16T10:00:00.0000Z"}
            }}
        }},
        {"selectedProfile", "Maple"},
        {"clientToken", "11451419198111451419198111451419"}
    };

    std::ofstream outfile(filePath);
    if (outfile.is_open()) {
        outfile << std::setw(4) << dummyProfiles << std::endl;
        outfile.close();
    }
    else {
        std::cerr << "Unable to open file for writing." << std::endl;
    }
}

void InstallForge(const std::string& minecraftDir, const std::string& version) {
    std::string dummyProfilesPath = ".minecraft/launcher_profiles.json";
    CreateDummyLauncherProfiles(dummyProfilesPath);
    const std::string forgeVersion = "1.12.2-14.23.5.2854";
    const std::string forgeUrl = "https://maven.minecraftforge.net/net/minecraftforge/forge/" + forgeVersion + "/forge-" + forgeVersion + "-installer.jar";
    const std::string forgeJarPath = minecraftDir + "forge-" + forgeVersion + "-installer.jar";
    std::cout << "Downloading Forge installer..." << std::endl;
    if (!DownloadFile(forgeUrl, forgeJarPath)) {
        std::cerr << "Failed to download Forge installer." << std::endl;
        return;
    }
    std::string absPath = fs::absolute(minecraftDir).string();
    std::string modifiedPath = absPath.substr(0, absPath.size() - 1);
    modifiedPath = "\"" + modifiedPath + "\"";
    std::string command = "java -jar " + forgeJarPath + " --installClient " + modifiedPath;
    std::cerr << "Running command:" + command << std::endl;
    std::thread installer_thread([command]() {
        int result = std::system(command.c_str());
        if (result == 0) {
            std::cout << "Forge installation completed successfully." << std::endl;
        }
        else {
            std::cerr << "Forge installation failed with error code: " << result << std::endl;
        }
        });
    installer_thread.join();
    fs::remove(forgeJarPath);
}

void VerifyAndDownloadResources(const std::string& minecraftDir, const std::string& version) {
    std::string versionJsonPath = minecraftDir + "versions/" + version + "/" + version + ".json";
    std::ifstream versionFile(versionJsonPath);
    if (!versionFile.is_open()) {
        std::cerr << "Failed to open version JSON file." << std::endl;
        return;
    }

    json versionInfo;
    versionFile >> versionInfo;
    versionFile.close();

    std::string clientJarPath = minecraftDir + "versions/" + version + "/" + version + ".jar";
    if (!fs::exists(clientJarPath)) {
        std::cout << "Client JAR missing. Downloading..." << std::endl;
        std::string clientUrl = versionInfo["downloads"]["client"]["url"];
        DownloadFile(clientUrl, clientJarPath);
    }

    std::string assetIndexId = versionInfo["assetIndex"]["id"];
    std::string assetIndexPath = minecraftDir + "assets/indexes/" + assetIndexId + ".json";
    if (!fs::exists(assetIndexPath)) {
        std::cout << "Asset index missing. Downloading..." << std::endl;
        std::string assetIndexUrl = versionInfo["assetIndex"]["url"];
        DownloadFile(assetIndexUrl, assetIndexPath);
    }

    std::ifstream assetIndexFile(assetIndexPath);
    json assetIndex;
    assetIndexFile >> assetIndex;
    assetIndexFile.close();

    for (const auto& [assetName, assetInfo] : assetIndex["objects"].items()) {
        std::string hash = assetInfo["hash"];
        std::string assetPath = minecraftDir + "assets/objects/" + hash.substr(0, 2) + "/" + hash;
        if (!fs::exists(assetPath)) {
            std::cout << "Asset missing: " << assetName << ". Downloading..." << std::endl;
            std::string assetUrl = "https://resources.download.minecraft.net/" + hash.substr(0, 2) + "/" + hash;
            DownloadFile(assetUrl, assetPath);
        }
    }

    for (const auto& library : versionInfo["libraries"]) {
        if (!library.contains("downloads") || !library["downloads"].contains("artifact")) {
            continue;
        }

        const auto& artifact = library["downloads"]["artifact"];
        if (!artifact.contains("path") || !artifact.contains("url")) {
            continue;
        }

        std::string path = artifact["path"];
        std::string url = artifact["url"];
        std::string fullPath = minecraftDir + "libraries/" + path;

        if (!fs::exists(fullPath)) {
            std::cout << "Library missing: " << path << ". Downloading..." << std::endl;
            DownloadFile(url, fullPath);
        }
    }

    std::cout << "Resource verification and download completed." << std::endl;
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
                    std::string fullNativePath = minecraftDir + "versions/" + version + "/" + version + "-natives/";
                    std::string jarPath = fullNativePath + fileName;

                    std::cout << "Downloading native library: " << fileName << std::endl;
                    if (DownloadFile(nativeUrl, jarPath)) {
                        if (ExtractJar(jarPath, fullNativePath)) {
                            std::cout << "Successfully extracted native library: " << fileName << std::endl;
                            fs::remove(jarPath);
                        }
                        else {
                            std::cerr << "Failed to extract native library: " << fileName << std::endl;
                        }
                    }
                    else {
                        std::cerr << "Failed to download native library: " << nativeUrl << std::endl;
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

    std::cout << "Minecraft " << version << " and its resources have been downloaded successfully!" << std::endl;
    VerifyAndDownloadResources(minecraftDir,version);
    InstallForge(minecraftDir, version);
}