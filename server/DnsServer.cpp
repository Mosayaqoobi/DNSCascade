/* --- DnsServer.cpp --- */

/* ------------------------------------------
author: undefined
date: 9/13/2026
------------------------------------------ */

#include "DnsServer.h"
#include "DnsMessage.h"
#include <iostream>

DnsServer::DnsServer(const int port) : port_(port) {
    socket_.bind(port_);
}

void DnsServer::run() {
    std::cout << "Listening on port: " << port_ << "\n";

    while (true) {
        std::string data {};
        std::string senderIp {};
        int senderPort {};
        if (!socket_.receiveFrom(data, senderIp, senderPort)) {
            continue;
        }

        DnsMessage query = DnsMessage::deserialize(data);
        DnsMessage response = handleQuery(query);

        socket_.sendTo(response.serialize(), senderIp, senderPort);
    }
}
