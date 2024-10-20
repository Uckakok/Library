#include "Defines.h"
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

std::string g_IpAddress = "172.30.46.162";
int g_Port = 49833;


bool readConfig() {
    std::ifstream configFile("config");
    if (!configFile.is_open()) {
        std::cerr << "Config file not found, using default values." << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(configFile, line)) {
        std::istringstream iss(line);
        std::string key;
        if (std::getline(iss, key, ':')) {
            std::string value;
            if (std::getline(iss, value)) {
                key.erase(std::remove_if(key.begin(), key.end(), ::isspace), key.end());
                value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
                if (key == "ServerAddress") {
                    g_IpAddress = value;
                }
                else if (key == "AppSocket") {
                    g_Port = std::stoi(value);
                }
            }
        }
    }

    return true;
}