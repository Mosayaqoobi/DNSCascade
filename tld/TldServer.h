/* --- TldServer.h --- */

/* ------------------------------------------
Author: undefined
Date: 9/13/2026
------------------------------------------ */

#ifndef TLDSERVER_H
#define TLDSERVER_H

#include "DnsServer.h"
#include "DnsMessage.h"
#include "ServerConfig.h"
#include <string>
#include <unordered_map>

/**
 * @brief One TLD's authority in the simulated hierarchy. Knows which
 * AuthoritativeServer is responsible for each domain registered under its
 * own TLD, and replies to every query with a referral to that server.
 *
 * One process per TLD: run the same binary once per config file.
 */
class TldServer : public DnsServer {
public:
    explicit TldServer(const std::string& cfgPath);

protected:
    DnsMessage handleQuery(const DnsMessage& query) override;

private:
    /**
     * @brief Loads "tld" and the "domains" table out of the config file.
     * @param cfgPath: path to the config file, e.g. "tld/cfg/a.json"
     */
    void init(const std::string& cfgPath);

    std::string tld_;                                   ///< the one TLD this server answers for, e.g. "a"
    std::unordered_map<std::string, Address> domains_;  ///< domain ("example.a") -> its AuthoritativeServer
};

#endif // TLDSERVER_H
