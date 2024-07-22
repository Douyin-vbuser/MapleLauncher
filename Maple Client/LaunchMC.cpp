#include <iostream>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <fstream>

namespace fs = std::filesystem;
using json = nlohmann::json;

const fs::path SETTINGS_PATH1 = fs::current_path() / "Maple" / "setting.json";
//#include "JsonUtil.cpp"

static std::string getExeDir() {
    std::string exePath = std::filesystem::current_path().string();
    std::filesystem::directory_iterator it(exePath);
    for (const auto& entry : it) {
        if (entry.is_directory() && entry.path().filename() == ".minecraft") {
            return entry.path().string();
        }
    }
    return "";
}

static void deleteLogsFolder() {
    std::filesystem::path logsPath = std::filesystem::current_path() / "logs";
    if (std::filesystem::exists(logsPath) && std::filesystem::is_directory(logsPath)) {
        std::filesystem::remove_all(logsPath);
        std::cout << "Deleted logs folder." << std::endl;
    }
    else {
        std::cout << "Logs folder does not exist." << std::endl;
    }
}

static void launchMinecraft(const std::string& javaPath, const std::string& playerName, const std::string& accessToken, const std::string& uuid) {
    std::string command = javaPath;

    command += " -XX:+UseG1GC -XX:-UseAdaptiveSizePolicy -XX:-OmitStackTraceInFastThrow "
        "-Dfml.ignoreInvalidMinecraftCertificates=True -Dfml.ignorePatchDiscrepancies=True "
        "-Dlog4j2.formatMsgNoLookups=true -XX:HeapDumpPath=MojangTricksIntelDriversForPerformance_javaw.exe_minecraft.exe.heapdump "
        "-Xmn138m -Xmx921m \"-Djava.library.path=" 
        + getExeDir() + "\\versions\\1.12.2-forge-14.23.5.2854\\1.12.2-forge-14.23.5.2854-natives\" "
        "-cp \"" + getExeDir() + "\\libraries\\com\\mojang\\patchy\\1.3.9\\patchy-1.3.9.jar;"
        + getExeDir() + "\\libraries\\oshi-project\\oshi-core\\1.1\\oshi-core-1.1.jar;"
        + getExeDir() + "\\libraries\\net\\java\\dev\\jna\\jna\\4.4.0\\jna-4.4.0.jar;"
        + getExeDir() + "\\libraries\\net\\java\\dev\\jna\\platform\\3.4.0\\platform-3.4.0.jar;"
        + getExeDir() + "\\libraries\\com\\ibm\\icu\\icu4j-core-mojang\\51.2\\icu4j-core-mojang-51.2.jar;"
        + getExeDir() + "\\libraries\\net\\sf\\jopt-simple\\jopt-simple\\5.0.3\\jopt-simple-5.0.3.jar;"
        + getExeDir() + "\\libraries\\com\\paulscode\\codecjorbis\\20101023\\codecjorbis-20101023.jar;"
        + getExeDir() + "\\libraries\\com\\paulscode\\codecwav\\20101023\\codecwav-20101023.jar;"
        + getExeDir() + "\\libraries\\com\\paulscode\\libraryjavasound\\20101123\\libraryjavasound-20101123.jar;"
        + getExeDir() + "\\libraries\\com\\paulscode\\librarylwjglopenal\\20100824\\librarylwjglopenal-20100824.jar;"
        + getExeDir() + "\\libraries\\com\\paulscode\\soundsystem\\20120107\\soundsystem-20120107.jar;"
        + getExeDir() + "\\libraries\\io\\netty\\netty-all\\4.1.9.Final\\netty-all-4.1.9.Final.jar;"
        + getExeDir() + "\\libraries\\com\\google\\guava\\guava\\21.0\\guava-21.0.jar;"
        + getExeDir() + "\\libraries\\org\\apache\\commons\\commons-lang3\\3.5\\commons-lang3-3.5.jar;"
        + getExeDir() + "\\libraries\\commons-io\\commons-io\\2.5\\commons-io-2.5.jar;"
        + getExeDir() + "\\libraries\\commons-codec\\commons-codec\\commons-codec-1.10.jar;"
        + getExeDir() + "\\libraries\\net\\java\\jinput\\jinput\\2.0.5\\jinput-2.0.5.jar;"
        + getExeDir() + "\\libraries\\net\\java\\jutils\\jutils\\1.0.0\\jutils-1.0.0.jar;"
        + getExeDir() + "\\libraries\\com\\google\\code\\gson\\gson\\2.8.0\\gson-2.8.0.jar;"
        + getExeDir() + "\\libraries\\com\\mojang\\authlib\\1.5.25\\authlib-1.5.25.jar;"
        + getExeDir() + "\\libraries\\com\\mojang\\realms\\1.10.22\\realms-1.10.22.jar;"
        + getExeDir() + "\\libraries\\org\\apache\\commons\\commons-compress\\1.8.1\\commons-compress-1.8.1.jar;"
        + getExeDir() + "\\libraries\\org\\apache\\httpcomponents\\httpclient\\4.3.3\\httpclient-4.3.3.jar;"
        + getExeDir() + "\\libraries\\commons-logging\\commons-logging\\1.1.3\\commons-logging-1.1.3.jar;"
        + getExeDir() + "\\libraries\\org\\apache\\httpcomponents\\httpcore\\4.3.2\\httpcore-4.3.2.jar;"
        + getExeDir() + "\\libraries\\it\\unimi\\dsi\\fastutil\\7.1.0\\fastutil-7.1.0.jar;"
        + getExeDir() + "\\libraries\\org\\apache\\logging\\log4j\\log4j-api\\2.8.1\\log4j-api-2.8.1.jar;"
        + getExeDir() + "\\libraries\\org\\apache\\logging\\log4j\\log4j-core\\2.8.1\\log4j-core-2.8.1.jar;"
        + getExeDir() + "\\libraries\\org\\lwjgl\\lwjgl\\lwjgl\\2.9.4-nightly-20150209\\lwjgl-2.9.4-nightly-20150209.jar;"
        + getExeDir() + "\\libraries\\org\\lwjgl\\lwjgl\\lwjgl_util\\2.9.4-nightly-20150209\\lwjgl_util-2.9.4-nightly-20150209.jar;"
        + getExeDir() + "\\libraries\\com\\mojang\\text2speech\\1.10.3\\text2speech-1.10.3.jar;"
        + getExeDir() + "\\libraries\\net\\minecraftforge\\forge\\1.12.2-14.23.5.2854\\forge-1.12.2-14.23.5.2854.jar;"
        + getExeDir() + "\\libraries\\org\\ow2\\asm\\asm-debug-all\\5.2\\asm-debug-all-5.2.jar;"
        + getExeDir() + "\\libraries\\net\\minecraft\\launchwrapper\\1.12\\launchwrapper-1.12.jar;"
        + getExeDir() + "\\libraries\\org\\jline\\jline\\3.5.1\\jline-3.5.1.jar;"
        + getExeDir() + "\\libraries\\com\\typesafe\\akka\\akka-actor_2.11\\2.3.3\\akka-actor_2.11-2.3.3.jar;"
        + getExeDir() + "\\libraries\\com\\typesafe\\config\\1.2.1\\config-1.2.1.jar;"
        + getExeDir() + "\\libraries\\org\\scala-lang\\scala-actors-migration_2.11\\1.1.0\\scala-actors-migration_2.11-1.1.0.jar;"
        + getExeDir() + "\\libraries\\org\\scala-lang\\scala-compiler\\2.11.1\\scala-compiler-2.11.1.jar;"
        + getExeDir() + "\\libraries\\org\\scala-lang\\plugins\\scala-continuations-library_2.11\\1.0.2_mc\\scala-continuations-library_2.11-1.0.2_mc.jar;"
        + getExeDir() + "\\libraries\\org\\scala-lang\\plugins\\scala-continuations-plugin_2.11.1\\1.0.2_mc\\scala-continuations-plugin_2.11.1-1.0.2_mc.jar;"
        + getExeDir() + "\\libraries\\org\\scala-lang\\scala-library\\2.11.1\\scala-library-2.11.1.jar;"
        + getExeDir() + "\\libraries\\org\\scala-lang\\scala-parser-combinators_2.11\\1.0.1\\scala-parser-combinators_2.11-1.0.1.jar;"
        + getExeDir() + "\\libraries\\org\\scala-lang\\scala-reflect\\2.11.1\\scala-reflect-2.11.1.jar;"
        + getExeDir() + "\\libraries\\org\\scala-lang\\scala-swing_2.11\\1.0.1\\scala-swing_2.11-1.0.1.jar;"
        + getExeDir() + "\\libraries\\org\\scala-lang\\scala-xml_2.11\\1.0.2\\scala-xml_2.11-1.0.2.jar;"
        + getExeDir() + "\\libraries\\lzma\\lzma\\0.0.1\\lzma-0.0.1.jar;"
        + getExeDir() + "\\libraries\\java3d\\vecmath\\1.5.2\\vecmath-1.5.2.jar;"
        + getExeDir() + "\\libraries\\net\\sf\\trove4j\\trove4j\\3.0.3\\trove4j-3.0.3.jar;"
        + getExeDir() + "\\libraries\\org\\apache\\maven\\maven-artifact\\3.5.3\\maven-artifact-3.5.3.jar;"
        + getExeDir() + "\\versions\\1.12.2\\1.12.2.jar\" "
        "net.minecraft.launchwrapper.Launch --username " + playerName + " --version 1.12.2-forge-14.23.5.2854 "
        "--gameDir \"" + getExeDir() + "\\.minecraft\" "
        "--assetsDir \"" + getExeDir() + "\\.minecraft\\assets\" "
        "--assetIndex 1.12 --uuid " + uuid + " "
        "--accessToken " + accessToken + " "
        "--userType msa --tweakClass net.minecraftforge.fml.common.launcher.FMLTweaker "
        "--versionType Forge --height 508 --width 873";
    system(command.c_str());
}

static std::string read1(const std::string& key) {
    if (!fs::exists(SETTINGS_PATH1)) {
        throw std::runtime_error("settings.json does not exist in Maple folder");
    }

    std::ifstream file(SETTINGS_PATH1);
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

static void runMinecraft() {
    std::string javaPath = "C:\\ProgramData\\BadlionClient\\jre1.8.0_51\\bin\\java.exe";
    std::string playerName = read1("name");
    std::string accessToken = read1("token");
    std::string uuid = read1("uuid");

    launchMinecraft(javaPath, playerName, accessToken, uuid);
    deleteLogsFolder();

}