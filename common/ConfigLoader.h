//
// Created by Mosa Yaqoobi on 2026-09-19.
//

#ifndef DNSCASCADE_CONFIGLOADER_H
#define DNSCASCADE_CONFIGLOADER_H

#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>
#include <string>

template <typename T>
T loadConfig(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Could not open config file: " + path);
    }
    nlohmann::json j;
    in >> j;
    return j.get<T>();
}

/**
 * @brief Reads just the "port" field of a server config. Servers need it in
 * their base-class initializer, before init() can load the rest of the file.
 */
inline int loadPort(const std::string& path) {
    return loadConfig<nlohmann::json>(path).at("port").get<int>();
}

#endif //DNSCASCADE_CONFIGLOADER_H
