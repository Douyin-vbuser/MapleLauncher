#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <sstream>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

const fs::path SETTINGS_PATH = fs::current_path() / "Maple" / "setting.json";


static std::string genUUID() {
    std::string uuid;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);

    std::stringstream ss;
    for (int i = 0; i < 32; i++) {
        int r = dis(gen);
        ss << std::hex << r;
    }

    uuid = ss.str();
    return uuid;
}

static bool existMCFolder() {
    fs::path mcPath = fs::current_path() / ".minecraft";
    return fs::exists(mcPath);
}

static void ensureMapleSettingsExist() {
    fs::path maplePath = fs::current_path() / "Maple";
    if (!fs::exists(maplePath)) {
        std::cout << "Maple folder does not exist. Creating it..." << std::endl;
        if (!fs::create_directory(maplePath)) {
            std::cerr << "Failed to create Maple folder." << std::endl;
        }
    }

    fs::path settingsPath = maplePath / "setting.json";
    if (!fs::exists(settingsPath)) {
        std::cout << "setting.json does not exist. Creating it..." << std::endl;
        std::string uuid = genUUID();
        json defaultSettings = {
            {"name","Traveller"},
            {"token","XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"},
            {"uuid",uuid}
        };

        std::ofstream settingsFile(settingsPath);
        if (!settingsFile.is_open()) {
            std::cerr << "Failed to create setting.json." << std::endl;
        }
        settingsFile << std::setw(4) << defaultSettings << std::endl;
        settingsFile.close();
    }

    std::cout << "Maple folder and setting.json are ready." << std::endl;
}

static std::string read(const std::string& key) {
    if (!fs::exists(SETTINGS_PATH)) {
        throw std::runtime_error("settings.json does not exist in Maple folder");
    }

    std::ifstream file(SETTINGS_PATH);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open settings.json");
    }

    json j;
    file >> j;

    if (j.find(key) != j.end() && j[key].is_string()) {
        return j[key].get<std::string>();
    }
    else {
        throw std::runtime_error("Key not found or value is not a string: " + key);
    }
}

static void write(const std::string& key, const std::string& value) {
    json j;
    if (fs::exists(SETTINGS_PATH)) {
        std::ifstream infile(SETTINGS_PATH);
        if (infile.is_open()) {
            infile >> j;
            infile.close();
        }
    }

    j[key] = value;

    std::ofstream outfile(SETTINGS_PATH);
    if (!outfile.is_open()) {
        throw std::runtime_error("Unable to open settings.json for writing");
    }
    outfile << std::setw(4) << j << std::endl;
}