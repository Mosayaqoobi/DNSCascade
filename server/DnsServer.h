/* --- DnsServer.h --- */

/* ------------------------------------------
Author: undefined
Date: 9/13/2026
------------------------------------------ */

#ifndef DNSSERVER_H
#define DNSSERVER_H

#include "DnsMessage.h"
#include "UdpSocket.h"

class DnsServer {
public:
    virtual ~DnsServer() = default;
    explicit DnsServer(int port);
    void run();

protected:
    virtual DnsMessage handleQuery(const DnsMessage& query) = 0;

private:
    UdpSocket socket_;
    int port_ = 0;

};

#endif // DNSSERVER_H
