/* --- DNSClient.cpp --- */

/* ------------------------------------------
author: undefined
date: 9/13/2026
------------------------------------------ */

#include "DnsClient.h"
#include "DnsMessage.h"
#include "DnsRecord.h"
#include "UdpSocket.h"
#include <algorithm>
#include <cctype>
#include <exception>
#include <optional>
#include <regex>
#include <iostream>

DnsClient::DnsClient(const ClientConfig& config) :
    resolverIp_(config.resolverIp), resolverPort_(config.resolverPort), maxRetries_(config.maxRetries) {
        sockfd_.setTimout(config.timeoutSeconds);
}

std::string DnsClient::enterHost() {
    std::string host;
    std::cout << "Enter a URL: ";
    std::cin >> host;
    while (!validateHost(host)) {
        if (!std::cin) {
            return ""; // stream exhausted (EOF/failure) -- nothing left to read
        }
        std::cout << "Please enter a valid URL. Starting with www... or https...\n";
        std::cin >> host;
    }
    return host;
}

bool DnsClient::validateHost(const std::string& host) {
    if (host.empty()) {
        return false;
    }
    static const std::regex urlPattern(
    R"(^(https?://)?(www\.)?[a-zA-Z0-9-]+(\.[a-zA-Z0-9-]+)*\.(com|org|net|test|a|b|c|d)\.?(/[a-zA-Z0-9.,_@%?&=~+#-]*)*$)",
    std::regex::icase
    );
    return std::regex_match(host, urlPattern);
}

std::string DnsClient::toHostname(const std::string& url) {
    std::string host = url;
    if (const auto scheme = host.find("://"); scheme != std::string::npos) {
        host.erase(0, scheme + 3);
    }
    if (const auto path = host.find('/'); path != std::string::npos) {
        host.erase(path);
    }
    if (!host.empty() && host.back() == '.') {
        host.pop_back();
    }
    std::ranges::transform(host, host.begin(),
                           [](const unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return host;
}

std::optional<DnsMessage> DnsClient::sendQuery(const std::string& host) {
    DnsMessage msg {};
    msg.header.id = nextId_++;
    msg.header.rd = true;
    msg.header.qdcount = 1;
    msg.questions = {Question{.qname = host, .qtype = RRType::A}};

    const std::string serializedMsg = msg.serialize();
    for (int attempt = 0; attempt <= maxRetries_; attempt++) {
        std::cout << "Attempt #" << attempt + 1 << " to send query to local resolver...\n";
        sockfd_.sendTo(serializedMsg, resolverIp_, resolverPort_);    
        std::string responseData {};
        std::string senderIp {};
        int senderPort {};

        // Skip past any late reply to an earlier query until ours arrives or we time out.
        while (sockfd_.receiveFrom(responseData, senderIp, senderPort)) {
            try {
                DnsMessage response = DnsMessage::deserialize(responseData);
                if (response.header.qr && response.header.id == msg.header.id) {
                    return response;
                }
            } catch (const std::exception& e) {
                std::cout << "Ignoring malformed reply: " << e.what() << "\n";
            }
        }
        std::cout << "Attempt #" << attempt + 1 << " timed out.\n";
    }
    return std::nullopt;
}

void DnsClient::displayResult(const std::string& host, const DnsMessage& response) {
    switch (response.header.rcode) {
        case Rcode::NOERROR:
            break;
        case Rcode::NXDOMAIN:
            std::cout << host << " does not exist (NXDOMAIN)\n";
            return;
        case Rcode::SERVFAIL:
            std::cout << "The resolver could not complete the lookup for " << host << " (SERVFAIL)\n";
            return;
        default:
            std::cout << "Could not resolve " << host << " (rcode "
                      << static_cast<int>(response.header.rcode) << ")\n";
            return;
    }

    bool foundAddress = false;
    for (const auto& rr : response.answers) {
        if (rr.type == RRType::CNAME) {
            std::cout << rr.name << " is an alias for " << rr.rdata << "\n";
        } else if (rr.type == RRType::A) {
            std::cout << rr.name << " -> " << rr.rdata << "  (ttl " << rr.ttl << "s)\n";
            foundAddress = true;
        }
    }
    if (!foundAddress) {
        std::cout << host << " exists but has no address record\n";
    }
}

void DnsClient::run() {
    while (true) {
        const std::string url = enterHost();
        if (url.empty()) {
            break; // input stream exhausted, nothing more to resolve
        }
        const std::string host = toHostname(url);

        if (auto response = sendQuery(host); !response.has_value()) {
            std::cout << "No response from resolver (request timed out)\n";
        } else {
            displayResult(host, response.value());
        }
        std::cout << "Resolve another? (y/n)\n";
        std::string again;
        std::cin >> again;
        if (again != "y" && again != "Y") {
            break;
        }
    }
}
