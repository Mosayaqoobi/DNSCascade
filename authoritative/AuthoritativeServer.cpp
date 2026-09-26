/* --- AuthoritativeServer.cpp --- */

/* ------------------------------------------
author: undefined
date: 9/13/2026
------------------------------------------ */

#include "AuthoritativeServer.h"
#include "ConfigLoader.h"
#include "DnsName.h"
#include <algorithm>
#include <exception>
#include <iostream>
#include <stdexcept>

namespace {
    // Guards against CNAME loops inside a zone (a -> b -> a).
    constexpr int kMaxCnameChain = 8;
}

AuthoritativeServer::AuthoritativeServer(const std::string& cfgPath) : DnsServer(loadPort(cfgPath)) {
    init(cfgPath);
}

void AuthoritativeServer::init(const std::string& cfgPath) {
    const auto cfg = loadConfig<nlohmann::json>(cfgPath);
    zone_ = DnsName::normalize(cfg.at("zone").get<std::string>());

    for (const auto& entry : cfg.at("records")) {
        ResourceRecord rr {
            .name = DnsName::normalize(entry.at("name").get<std::string>()),
            .type = rrTypeFromString(entry.at("type").get<std::string>()),
            .ttl = entry.at("ttl").get<uint32_t>(),
            .rdata = entry.at("rdata").get<std::string>()
        };
        if (!DnsName::isInZone(rr.name, zone_)) {
            throw std::runtime_error("Record " + rr.name + " is outside zone " + zone_);
        }
        if (rr.type == RRType::CNAME) {
            rr.rdata = DnsName::normalize(rr.rdata);
        }
        records_[rr.name].push_back(std::move(rr));
    }
}

DnsMessage AuthoritativeServer::handleQuery(const DnsMessage& query) {
    DnsMessage response = makeResponse(query);
    if (query.questions.empty()) {
        response.header.rcode = Rcode::FORMERR;
        return response;
    }

    const std::string qname = DnsName::normalize(query.questions[0].qname);
    if (!DnsName::isInZone(qname, zone_)) {
        response.header.rcode = Rcode::REFUSED; // not our zone -- a TLD shouldn't have sent this here
        return response;
    }

    response.header.aa = true;
    answer(response, qname, query.questions[0].qtype);
    return response;
}

void AuthoritativeServer::answer(DnsMessage& response, const std::string& qname, const RRType qtype) const {
    std::string name = qname;

    for (int depth = 0; depth < kMaxCnameChain; ++depth) {
        const auto it = records_.find(name);
        if (it == records_.end()) {
            if (response.answers.empty()) {
                response.header.rcode = Rcode::NXDOMAIN;
            }
            return;
        }

        const auto& rrs = it->second;
        bool matched = false;
        for (const auto& rr : rrs) {
            if (rr.type == qtype) {
                response.answers.push_back(rr);
                matched = true;
            }
        }
        if (matched) {
            return;
        }

        const auto cname = std::ranges::find(rrs, RRType::CNAME, &ResourceRecord::type);
        if (cname == rrs.end()) {
            return; // NODATA: the name exists, just not with this type
        }
        response.answers.push_back(*cname);
        name = cname->rdata;
        if (!DnsName::isInZone(name, zone_)) {
            return; // alias leaves our zone; the resolver follows it from here
        }
    }
    response.header.rcode = Rcode::SERVFAIL; // CNAME loop
}

int main(const int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: authoritative_server <config_path>   e.g. authoritative_server authoritative/cfg/example.a.json\n";
        return 1;
    }
    try {
        AuthoritativeServer server(argv[1]);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "authoritative_server: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
