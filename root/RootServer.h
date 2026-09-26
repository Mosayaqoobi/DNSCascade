/* --- RootServer.h --- */

/* ------------------------------------------
Author: undefined
Date: 9/13/2026
------------------------------------------ */

#ifndef ROOTSERVER_H
#define ROOTSERVER_H

#include "DnsServer.h"
#include "DnsMessage.h"
#include "ServerConfig.h"
#include <string>
#include <unordered_map>

/**
 * @brief Root of the simulated DNS hierarchy. Knows nothing about individual
 * domains -- only which TldServer is responsible for each TLD it's configured
 * with, and replies to every query with a referral to that server.
 */
class RootServer : public DnsServer {
public:
    explicit RootServer(const std::string& cfgPath);

protected:
    DnsMessage handleQuery(const DnsMessage& query) override;

private:
    /**
     * @brief Loads the "tlds" table out of the config file into tlds_.
     * @param cfgPath: path to the config file, e.g. "root/cfg/root.json"
     */
    void init(const std::string& cfgPath);

    std::unordered_map<std::string, Address> tlds_;  ///< tld ("a") -> address of its TldServer
};

#endif // ROOTSERVER_H
