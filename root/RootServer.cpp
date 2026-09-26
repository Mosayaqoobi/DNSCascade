/* --- RootServer.cpp --- */

/* ------------------------------------------
author: undefined
date: 9/13/2026
------------------------------------------ */

#include "RootServer.h"
#include "ConfigLoader.h"
#include "DnsName.h"
#include "DnsRecord.h"
#include <exception>
#include <iostream>

RootServer::RootServer(const std::string& cfgPath) : DnsServer(loadPort(cfgPath)) {
    init(cfgPath);
}

void RootServer::init(const std::string& cfgPath) {
    const auto cfg = loadConfig<nlohmann::json>(cfgPath); // must outlive the loop below
    for (const auto& [tld, addr] : cfg.at("tlds").items()) {
        tlds_[DnsName::normalize(tld)] = addr.get<Address>();
    }
}

DnsMessage RootServer::handleQuery(const DnsMessage& query) {
    DnsMessage response = makeResponse(query);
    if (query.questions.empty()) {
        response.header.rcode = Rcode::FORMERR;
        return response;
    }

    const std::string tld = DnsName::lastLabels(DnsName::normalize(query.questions[0].qname), 1);
    const auto it = tlds_.find(tld);
    if (it == tlds_.end()) {
        response.header.rcode = Rcode::NXDOMAIN; // no TLD server configured for this suffix
        return response;
    }

    addReferral(response, tld, it->second);
    return response;
}

int main(const int argc, char* argv[]) {
    const std::string cfgPath = argc > 1 ? argv[1] : "root/cfg/root.json";
    try {
        RootServer server(cfgPath);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "root_server: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
