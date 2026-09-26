/* --- LocalResolver.h --- */

/* ------------------------------------------
Author: undefined
Date: 9/13/2026
------------------------------------------ */

#ifndef LOCALRESOLVER_H
#define LOCALRESOLVER_H

#include "DnsMessage.h"
#include "DnsServer.h"
#include "DnsCache.h"
#include "ServerConfig.h"
#include "UdpSocket.h"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

/**
 * @brief The client's one point of contact. Answers from cache when it can,
 * otherwise walks root -> tld -> authoritative on the client's behalf and
 * caches what it learns.
 */
class LocalResolver : public DnsServer {
public:
    explicit LocalResolver(const std::string& cfgPath);

protected:
    DnsMessage handleQuery(const DnsMessage& query) override;

private:
    struct Resolution {
        uint8_t rcode;
        std::vector<ResourceRecord> answers;
    };

    /**
     * @brief Loads the root address and outbound timeout/retry settings.
     * @param cfgPath: path to the config file, e.g. "resolver/cfg/resolver.json"
     */
    void init(const std::string& cfgPath);

    /**
     * @brief Cache first, then iterate(); chases CNAMEs that point into other zones.
     */
    Resolution resolve(const std::string& qname, RRType qtype, int depth);

    /**
     * @brief One full walk down the hierarchy starting at root, following referrals.
     */
    Resolution iterate(const std::string& qname, RRType qtype);

    /**
     * @brief Send a query to one server and wait for its matching reply, retrying on timeout.
     */
    [[nodiscard]] std::optional<DnsMessage> queryServer(const DnsMessage& query, const Address& server) const;

    /**
     * @brief Pull the next server's address out of a referral's NS + glue records.
     */
    static std::optional<Address> nextHop(const DnsMessage& referral);

    UdpSocket outboundSocket_;  ///< its own socket, separate from the base class's listening socket
    DnsCache cache_;            ///< answers learned from authoritative servers, until their ttl runs out
    Address root_ {};           ///< where every walk down the hierarchy starts
    uint16_t nextId_ = 0;       ///< id for the next outbound query
    int maxRetries_ = 1;        ///< resends per upstream server after the first attempt times out
};

#endif // LOCALRESOLVER_H
