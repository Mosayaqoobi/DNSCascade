/* --- AuthoritativeServer.h --- */

/* ------------------------------------------
Author: undefined
Date: 9/13/2026
------------------------------------------ */

#ifndef AUTHORITATIVESERVER_H
#define AUTHORITATIVESERVER_H

#include "DnsServer.h"
#include "DnsMessage.h"
#include "DnsRecord.h"
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief The bottom of the hierarchy: the only server that actually holds
 * records. Owns exactly one zone (e.g. "example.a") and answers for any name
 * inside it with aa set -- either the records, NODATA, or NXDOMAIN.
 *
 * One process per zone: run the same binary once per config file.
 */
class AuthoritativeServer : public DnsServer {
public:
    explicit AuthoritativeServer(const std::string& cfgPath);

protected:
    DnsMessage handleQuery(const DnsMessage& query) override;

private:
    /**
     * @brief Loads "zone" and the "records" list out of the config file.
     * @param cfgPath: path to the config file, e.g. "authoritative/cfg/example.a.json"
     */
    void init(const std::string& cfgPath);

    /**
     * @brief Fills response.answers for (qname, qtype), following CNAMEs that
     * stay inside this zone. A CNAME pointing outside the zone is returned as-is
     * for the resolver to chase.
     */
    void answer(DnsMessage& response, const std::string& qname, RRType qtype) const;

    std::string zone_;                                                   ///< e.g. "example.a"
    std::unordered_map<std::string, std::vector<ResourceRecord>> records_; ///< owner name -> its records
};

#endif // AUTHORITATIVESERVER_H
