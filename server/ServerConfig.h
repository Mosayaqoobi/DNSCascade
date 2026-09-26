/* --- ServerConfig.h --- */

#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <nlohmann/json.hpp>
#include <string>

/**
 * @brief A network address, used wherever a config needs to point at another
 * server (root's tlds table, tld's domains table, resolver's root address).
 */
struct Address {
    std::string ip;
    int port;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Address, ip, port)
};

#endif // SERVERCONFIG_H
