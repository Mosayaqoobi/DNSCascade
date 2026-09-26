/* --- TldServer.cpp --- */

/* ------------------------------------------
author: undefined
date: 9/13/2026
------------------------------------------ */

#include "TldServer.h"
#include "ConfigLoader.h"
#include "DnsName.h"
#include "DnsRecord.h"
#include <exception>
#include <iostream>

TldServer::TldServer(const std::string& cfgPath) : DnsServer(loadPort(cfgPath)) {
    init(cfgPath);
}

void TldServer::init(const std::string& cfgPath) {
    const auto cfg = loadConfig<nlohmann::json>(cfgPath);
    tld_ = DnsName::normalize(cfg.at("tld").get<std::string>());
    for (const auto& [domain, addr] : cfg.at("domains").items()) {
        domains_[DnsName::normalize(domain)] = addr.get<Address>();
    }
}

DnsMessage TldServer::handleQuery(const DnsMessage& query) {
    DnsMessage response = makeResponse(query);
    if (query.questions.empty()) {
        response.header.rcode = Rcode::FORMERR;
        return response;
    }

    const std::string qname = DnsName::normalize(query.questions[0].qname);
    if (DnsName::lastLabels(qname, 1) != tld_) {
        response.header.rcode = Rcode::REFUSED; // not our TLD -- root shouldn't have sent this here
        return response;
    }

    const std::string domain = DnsName::lastLabels(qname, 2);
    const auto it = domains_.find(domain);
    if (it == domains_.end()) {
        response.header.rcode = Rcode::NXDOMAIN; // nothing registered under this name
        return response;
    }

    addReferral(response, domain, it->second);
    return response;
}

int main(const int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: tld_server <config_path>   e.g. tld_server tld/cfg/a.json\n";
        return 1;
    }
    try {
        TldServer server(argv[1]);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "tld_server: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
