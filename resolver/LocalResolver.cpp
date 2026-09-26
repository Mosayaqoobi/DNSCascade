/* --- LocalResolver.cpp --- */

/* ------------------------------------------
author: undefined
date: 9/13/2026
------------------------------------------ */

#include "LocalResolver.h"
#include "ConfigLoader.h"
#include "DnsName.h"
#include <algorithm>
#include <charconv>
#include <exception>
#include <iostream>
#include <ranges>

namespace {
    // root -> tld -> authoritative is 3; the headroom covers deeper delegations.
    constexpr int kMaxHops = 8;
    // Guards against CNAME loops that span zones (a.example.a -> b.example.b -> a.example.a).
    constexpr int kMaxCnameDepth = 8;
}

LocalResolver::LocalResolver(const std::string& cfgPath) : DnsServer(loadPort(cfgPath)) {
    init(cfgPath);
}

void LocalResolver::init(const std::string& cfgPath) {
    const auto cfg = loadConfig<nlohmann::json>(cfgPath);
    root_ = cfg.at("root").get<Address>();
    maxRetries_ = cfg.at("maxRetries").get<int>();
    outboundSocket_.setTimout(cfg.at("timeoutSeconds").get<int>());
}

DnsMessage LocalResolver::handleQuery(const DnsMessage& query) {
    DnsMessage response = makeResponse(query);
    response.header.ra = true;
    if (query.questions.empty()) {
        response.header.rcode = Rcode::FORMERR;
        return response;
    }

    auto [rcode, answers] = resolve(query.questions[0].qname, query.questions[0].qtype, 0);
    response.header.rcode = rcode;
    response.answers = std::move(answers);
    return response;
}

LocalResolver::Resolution LocalResolver::resolve(const std::string& qname, const RRType qtype, const int depth) {
    if (depth > kMaxCnameDepth) {
        return {Rcode::SERVFAIL, {}};
    }
    const std::string name = DnsName::normalize(qname);

    if (auto cached = cache_.lookup(name, qtype)) {
        return {Rcode::NOERROR, {std::move(*cached)}};
    }
    if (qtype != RRType::CNAME) {
        if (auto alias = cache_.lookup(name, RRType::CNAME)) {
            Resolution target = resolve(alias->rdata, qtype, depth + 1);
            target.answers.insert(target.answers.begin(), std::move(*alias));
            return target;
        }
    }

    Resolution result = iterate(name, qtype);
    if (result.rcode != Rcode::NOERROR) {
        return result;
    }
    for (const auto& rr : result.answers) {
        cache_.insert(rr);
    }

    // The authoritative server follows aliases inside its own zone; if the chain
    // leaves the zone we get the CNAME back without a final answer and must
    // start a fresh walk for the target.
    const bool answered = std::ranges::any_of(result.answers,
                                              [qtype](const ResourceRecord& rr) { return rr.type == qtype; });
    if (!answered && qtype != RRType::CNAME) {
        const auto lastAlias = std::ranges::find(result.answers | std::views::reverse,
                                                 RRType::CNAME, &ResourceRecord::type);
        if (lastAlias != (result.answers | std::views::reverse).end()) {
            Resolution target = resolve(lastAlias->rdata, qtype, depth + 1);
            result.rcode = target.rcode;
            result.answers.insert(result.answers.end(), target.answers.begin(), target.answers.end());
        }
    }
    return result;
}

LocalResolver::Resolution LocalResolver::iterate(const std::string& qname, const RRType qtype) {
    DnsMessage upstream {};
    upstream.header.id = nextId_++;
    upstream.header.rd = false; // iterative: we do the walking, servers just refer us onward
    upstream.header.qdcount = 1;
    upstream.questions = {Question{.qname = qname, .qtype = qtype}};

    Address server = root_;
    for (int hop = 0; hop < kMaxHops; ++hop) {
        const std::optional<DnsMessage> reply = queryServer(upstream, server);
        if (!reply) {
            std::cerr << "No reply from " << server.ip << ":" << server.port << " for " << qname << std::endl;
            return {Rcode::SERVFAIL, {}};
        }

        if (reply->header.rcode == Rcode::NXDOMAIN) {
            return {Rcode::NXDOMAIN, {}};
        }
        if (reply->header.rcode != Rcode::NOERROR) {
            return {Rcode::SERVFAIL, {}}; // REFUSED/SERVFAIL upstream means the hierarchy is misconfigured
        }
        if (reply->header.aa || !reply->answers.empty()) {
            return {Rcode::NOERROR, reply->answers}; // empty + aa is NODATA: name exists, no record of this type
        }

        const std::optional<Address> next = nextHop(*reply);
        if (!next) {
            return {Rcode::SERVFAIL, {}}; // neither an answer nor a usable referral
        }
        server = *next;
    }
    return {Rcode::SERVFAIL, {}};
}

std::optional<DnsMessage> LocalResolver::queryServer(const DnsMessage& query, const Address& server) const {
    const std::string serializedMsg = query.serialize();

    for (int attempt = 0; attempt <= maxRetries_; ++attempt) {
        outboundSocket_.sendTo(serializedMsg, server.ip, server.port);

        std::string responseData {};
        std::string senderIp {};
        int senderPort {};
        // Keep reading past stale replies (e.g. a late answer to an earlier
        // attempt) until our id shows up or the socket times out.
        while (outboundSocket_.receiveFrom(responseData, senderIp, senderPort)) {
            try {
                DnsMessage response = DnsMessage::deserialize(responseData);
                if (response.header.qr && response.header.id == query.header.id) {
                    return response;
                }
            } catch (const std::exception& e) {
                std::cerr << "Ignoring malformed reply from " << senderIp << ":" << senderPort
                          << ": " << e.what() << std::endl;
            }
        }
    }
    return std::nullopt;
}

std::optional<Address> LocalResolver::nextHop(const DnsMessage& referral) {
    const auto parse = [](const std::string& rdata) -> std::optional<Address> {
        const auto colon = rdata.rfind(':');
        if (colon == std::string::npos || colon == 0) {
            return std::nullopt;
        }
        int port = 0;
        const char* begin = rdata.data() + colon + 1;
        const char* end = rdata.data() + rdata.size();
        if (auto [ptr, ec] = std::from_chars(begin, end, port); ec != std::errc{} || ptr != end ||
            port <= 0 || port > 65535) {
            return std::nullopt;
        }
        return Address{.ip = rdata.substr(0, colon), .port = port};
    };

    // Prefer the glue record for a nameserver the authority section actually names.
    for (const auto& ns : referral.authority) {
        if (ns.type != RRType::NS) continue;
        for (const auto& glue : referral.additional) {
            if (glue.type == RRType::A && DnsName::normalize(glue.name) == DnsName::normalize(ns.rdata)) {
                if (auto addr = parse(glue.rdata)) return addr;
            }
        }
    }
    for (const auto& glue : referral.additional) {
        if (glue.type == RRType::A) {
            if (auto addr = parse(glue.rdata)) return addr;
        }
    }
    return std::nullopt;
}

int main(const int argc, char* argv[]) {
    const std::string cfgPath = argc > 1 ? argv[1] : "resolver/cfg/resolver.json";
    try {
        LocalResolver resolver(cfgPath);
        resolver.run();
    } catch (const std::exception& e) {
        std::cerr << "resolver: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
