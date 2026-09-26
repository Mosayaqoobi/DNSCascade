/* --- DnsServer.cpp --- */

/* ------------------------------------------
author: undefined
date: 9/13/2026
------------------------------------------ */

#include "DnsServer.h"
#include "DnsMessage.h"
#include <exception>
#include <iostream>

DnsServer::DnsServer(const int port) : port_(port) {
    socket_.bind(port_);
}

void DnsServer::run() {
    std::cout << "Listening on port: " << port_ << std::endl;

    while (true) {
        std::string data {};
        std::string senderIp {};
        int senderPort {};
        if (!socket_.receiveFrom(data, senderIp, senderPort)) {
            continue;
        }

        try {
            const DnsMessage query = DnsMessage::deserialize(data);
            DnsMessage response = handleQuery(query);

            response.header.qdcount = static_cast<uint16_t>(response.questions.size());
            response.header.ancount = static_cast<uint16_t>(response.answers.size());
            response.header.nscount = static_cast<uint16_t>(response.authority.size());
            response.header.arcount = static_cast<uint16_t>(response.additional.size());

            socket_.sendTo(response.serialize(), senderIp, senderPort);
        } catch (const std::exception& e) {
            // One bad packet shouldn't take the whole server down.
            std::cerr << "Dropping packet from " << senderIp << ":" << senderPort
                      << ": " << e.what() << std::endl;
        }
    }
}

DnsMessage DnsServer::makeResponse(const DnsMessage& query) {
    DnsMessage response {};
    response.header.id = query.header.id;
    response.header.opcode = query.header.opcode;
    response.header.rd = query.header.rd;
    response.header.qr = true;
    response.header.rcode = Rcode::NOERROR;
    response.questions = query.questions;
    return response;
}

void DnsServer::addReferral(DnsMessage& response, const std::string& zone, const Address& next) {
    const std::string nsName = "ns." + zone;
    response.header.rcode = Rcode::NOERROR;
    response.header.aa = false;
    response.authority = { ResourceRecord{ .name = zone, .type = RRType::NS, .ttl = 3600, .rdata = nsName } };
    response.additional = { ResourceRecord{
        .name = nsName,
        .type = RRType::A,
        .ttl = 3600,
        .rdata = next.ip + ":" + std::to_string(next.port)
    } };
}
