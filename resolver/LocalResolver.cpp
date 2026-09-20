/* --- LocalResolver.cpp --- */

/* ------------------------------------------
author: undefined
date: 9/13/2026
------------------------------------------ */

#include "LocalResolver.h"
#include "DnsMessage.h"
#include <optional>

LocalResolver::LocalResolver(const int port, std::string rootIp, const int rootPort) :
    DnsServer(port), rootIp_(std::move(rootIp)), rootPort_(rootPort), nextId_(0), maxRetries_(3) {
        outboundSocket_.setTimout(3);
}

std::optional<DnsMessage> LocalResolver::queryServer(const DnsMessage& query, const std::string& ip, int port) const {
    const std::string serializedMsg = query.serialize();

    for (int attempt = 0; attempt < maxRetries_; attempt++) {
        outboundSocket_.sendTo(serializedMsg, ip, port);
        std::string responseData {};
        std::string senderIp {};
        int senderPort {};
        if (!outboundSocket_.receiveFrom(responseData, senderIp, senderPort)) {
            continue;
        }

        DnsMessage response = DnsMessage::deserialize(responseData);
        if (response.header.id != query.header.id) {
            continue;
        }
        return response;
    }
    return std::nullopt;
}

DnsMessage LocalResolver::handleQuery(const DnsMessage& query) {
    if (query.questions.empty()) {
        DnsMessage response;
        response.header = query.header;
        response.header.qr = true;
        response.header.rcode = 1;
        return response;
    }

    const std::string qname = query.questions[0].qname;
    const RRType qtype = query.questions[0].qtype;

    if (auto cached = cache_.lookup(qname, qtype)) {
        DnsMessage response {
            .header = query.header,
            .header.qr = true,
            .header.rcode = 0,
            .header.ancount = 1,
            .questions = query.questions,
            .answers = {std::move( *cached )},
        };
        return response;
    }

    DnsMessage upstreamQuery {};
    upstreamQuery.header = DnsHeader {
        .qdcount = 1,
        .ancount = 0,
        .nscount = 0,
        .arcount = 0,
        .id = nextId_++,
        .rcode = 0,
        .opcode = 0,
        .qr = false,
        .aa = false,
        .tc = false,
        .rd = true,
        .ra = true
    };
    upstreamQuery.questions = query.questions;

    std::string nextIp = rootIp_;
    int nextPort = rootPort_;

    DnsMessage failResponse {
        .header = query.header,
        .header.qr = true,
        .header.rcode = 2
    };

    for (int hop = 0; hop < 3; hop++) {
        std::optional<DnsMessage> reply = queryServer(upstreamQuery, nextIp, nextPort);
        if (!reply.has_value()) {
            return failResponse;
        }

        if (!reply->answers.empty()) {
            for (const auto& rr : reply->answers) {
                cache_.insert(rr);
            }
            
            DnsMessage response {
                .header = query.header,
                .header.qr = true,
                .header.rcode = reply->header.rcode,
                .header.ancount = static_cast<uint16_t>(reply->answers.size()),
                .questions = query.questions,
                .answers = std::move(reply->answers),
            };
            return response;
        }
        if (reply->header.rcode != 0 || reply->additional.empty()) {
            failResponse.header.rcode = reply->header.rcode != 0? reply->header.rcode : 2;
            return failResponse;
        }
        const std::string& glue = reply->additional[0].rdata;
        auto colonPos = glue.find(':');
        if (colonPos == std::string::npos) {
            return failResponse;
        }
        nextIp = glue.substr(0, colonPos);
        nextPort = std::stoi(glue.substr(colonPos + 1));
    }
    return failResponse;
}
