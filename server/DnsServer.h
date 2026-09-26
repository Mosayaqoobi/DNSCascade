/* --- DnsServer.h --- */

/* ------------------------------------------
Author: undefined
Date: 9/13/2026
------------------------------------------ */

#ifndef DNSSERVER_H
#define DNSSERVER_H

#include "DnsMessage.h"
#include "ServerConfig.h"
#include "UdpSocket.h"
#include <string>

class DnsServer {
public:
    virtual ~DnsServer() = default;
    explicit DnsServer(int port);

    /**
     * @brief Receive a query, hand it to handleQuery, send the reply back to the
     * sender. Loops forever.
     */
    void run();

protected:
    virtual DnsMessage handleQuery(const DnsMessage& query) = 0;

    /**
     * @brief A response skeleton for `query`: same id and questions, qr set,
     * everything else cleared. Section counts are filled in by run().
     */
    static DnsMessage makeResponse(const DnsMessage& query);

    /**
     * @brief Turn `response` into a referral: "I'm not authoritative for this, ask
     * the server for `zone` at `next`". The next hop's address rides in the glue
     * record's rdata as "ip:port", since every simulated server has its own port.
     */
    static void addReferral(DnsMessage& response, const std::string& zone, const Address& next);

private:
    UdpSocket socket_;
    int port_ = 0;
};

#endif // DNSSERVER_H
