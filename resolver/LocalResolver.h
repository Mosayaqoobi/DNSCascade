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
#include "UdpSocket.h"
#include <optional>
#include <string>

class LocalResolver : public DnsServer {
public:
    LocalResolver(int port, std::string rootIp, int rootPort);

protected:
    DnsMessage handleQuery(const DnsMessage& query) override;


private:
    [[nodiscard]] std::optional<DnsMessage> queryServer(const DnsMessage& query, const std::string& ip, int port) const;

    UdpSocket outboundSocket_;  ///< its own socket, seperate from base class socket_
    DnsCache cache_;    ///< a storage class meant to store conversions hsot -> ip
    std::string rootIp_;    ///< a fixed root server ip
    int rootPort_;          ///< a fixed root server port
    uint16_t nextId_;       ///< the next id of the query
    int maxRetries_;        ///< number of tries to send to the root server

};

#endif // LOCALRESOLVER_H
